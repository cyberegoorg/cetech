//==============================================================================
// Includes
//==============================================================================

#include <celib/thread/thread.h>
#include <celib/memory/memory.h>
#include <engine/memory/memsys.h>
#include <celib/cpu/cpu.h>

#include <engine/develop/develop_system.h>
#include <engine/plugin/plugin_api.h>

#include "task_queue.h"
#include "../task.h"


//==============================================================================
// Defines
//==============================================================================

#define make_task(i) (task_t){.id = i}

#define MAX_TASK 4096
#define LOG_WHERE "taskmanager"

//==============================================================================
// Globals
//==============================================================================

struct task {
    void *data;
    task_work_t task_work;
    const char *name;
    enum task_affinity affinity;
};

static __thread char _worker_id = 0;

static struct G {
    struct task _task_pool[MAX_TASK];
    struct task_queue _workers_queue[TASK_MAX_WORKERS];
    thread_t _workers[TASK_MAX_WORKERS - 1];
    struct task_queue _gloalQueue;
    atomic_int _task_pool_idx;
    int _workers_count;
    int _Run;
} TaskManagerGlobal = {0};

#define _G TaskManagerGlobal

//==============================================================================
// Private
//==============================================================================

typedef struct {
    u32 id;
} task_t;

static const task_t task_null = (task_t) {.id = 0};

static task_t _new_task() {
    int idx = atomic_fetch_add(&_G._task_pool_idx, 1);

    if ((idx & (MAX_TASK - 1)) == 0) {
        idx = atomic_fetch_add(&_G._task_pool_idx, 1);
    }

    return make_task(idx & (MAX_TASK - 1));
}

static void _push_task(task_t t) {
    CEL_ASSERT("", t.id != 0);

    int affinity = _G._task_pool[t.id].affinity;

    struct task_queue *q;
    switch (_G._task_pool[t.id].affinity) {
        case TASK_AFFINITY_NONE:
            q = &_G._gloalQueue;
            break;

        default:
            q = &_G._workers_queue[affinity - 2];
            break;
    }

    queue_task_push(q, t.id);
}


static task_t _try_pop(struct task_queue *q) {
    u32 poped_task;

    if (!queue_task_size(q)) {
        return task_null;
    }

    if (queue_task_pop(q, &poped_task, 0)) {
        if (poped_task != 0) {
            return make_task(poped_task);
        }
    }

    return task_null;
}

static void _mark_task_job_done(task_t task) {

}

static task_t _task_pop_new_work() {
    task_t popedTask;

    struct task_queue *qw = &_G._workers_queue[_worker_id];
    struct task_queue *qg = &_G._gloalQueue;

    popedTask = _try_pop(qw);
    if (popedTask.id != 0) {
        return popedTask;
    }

    popedTask = _try_pop(qg);
    if (popedTask.id != 0) {
        return popedTask;
    }

    return task_null;
}

int taskmanager_do_work();

static int _task_worker(void *o) {
    // Wait for run signal 0 -> 1
    while (!_G._Run) {
    }

    _worker_id = (char) o;

    log_debug("task_worker", "Worker %d init", _worker_id);

    while (_G._Run) {
        if (!taskmanager_do_work()) {
            cel_thread_yield();
        }
    }

    log_debug("task_worker", "Worker %d shutdown", _worker_id);
    return 1;
}

IMPORT_API(DevelopSystemApi, 0);
IMPORT_API(MemSysApi, 0);

static void _init(get_api_fce_t get_engine_api) {
    INIT_API(DevelopSystemApi, DEVELOP_SERVER_API_ID, 0);
    INIT_API(MemSysApi, MEMORY_API_ID, 0);

    _G = (struct G) {0};

    int core_count = cel_cpu_count();

    static const uint32_t main_threads_count = 1;
    const uint32_t worker_count = core_count - main_threads_count;

    log_info("task", "Core/Main/Worker: %d, %d, %d", core_count,
             main_threads_count, worker_count);

    _G._workers_count = worker_count;

    queue_task_init(&_G._gloalQueue, MAX_TASK, MemSysApiV0.main_allocator());

    for (int i = 0; i < worker_count + 1; ++i) {
        queue_task_init(&_G._workers_queue[i], MAX_TASK,
                        MemSysApiV0.main_allocator());
    }

    for (int j = 0; j < worker_count; ++j) {
        _G._workers[j] = cel_thread_create((thread_fce_t) _task_worker,
                                           "worker",
                                           (void *) ((intptr_t) (j + 1)));
    }

    _G._Run = 1;
}

static void _shutdown() {
    _G._Run = 0;

    int status = 0;

    for (u32 i = 0; i < _G._workers_count; ++i) {
        //cel_thread_kill(_G._workers[i]);
        cel_thread_wait(_G._workers[i], &status);
    }

    queue_task_destroy(&_G._gloalQueue);

    for (int i = 0; i < _G._workers_count + 1; ++i) {
        queue_task_destroy(&_G._workers_queue[i]);
    }

    _G = (struct G) {0};
}

//==============================================================================
// Interface
//==============================================================================

void taskmanager_add(struct task_item *items,
                     u32 count) {
    for (u32 i = 0; i < count; ++i) {
        task_t task = _new_task();
        _G._task_pool[task.id] = (struct task) {
                .name = items[i].name,
                .task_work = items[i].work,
                .affinity = items[i].affinity,
        };

        _G._task_pool[task.id].data = items[i].data;

        _push_task(task);
    }
}

int taskmanager_do_work() {
    task_t t = _task_pop_new_work();

    if (t.id == 0) {
        return 0;
    }

    struct scope_data sd = DevelopSystemApiV0.enter_scope(
            _G._task_pool[t.id].name);

    _G._task_pool[t.id].task_work(_G._task_pool[t.id].data);

    DevelopSystemApiV0.leave_scope(sd);

    _mark_task_job_done(t);

    return 1;
}

void taskmanager_wait_atomic(atomic_int *signal,
                             u32 value) {
    while (atomic_load_explicit(signal, memory_order_acquire) == value) {
        taskmanager_do_work();
    }
}

char taskmanager_worker_id() {
    return _worker_id;
}

int taskmanager_worker_count() {
    return _G._workers_count;
}

void *task_get_plugin_api(int api,
                          int version) {


    switch (api) {
        case PLUGIN_EXPORT_API_ID:
            switch (version) {
                case 0: {
                    static struct plugin_api_v0 plugin = {0};

                    plugin.init = _init;
                    plugin.shutdown = _shutdown;

                    return &plugin;
                }

                default:
                    return NULL;
            };
        case TASK_API_ID:
            switch (version) {
                case 0: {
                    static struct TaskApiV0 api = {
                            .worker_count = taskmanager_worker_count,
                            .add = taskmanager_add,
                            .do_work = taskmanager_do_work,
                            .wait_atomic = taskmanager_wait_atomic,
                            .worker_id = taskmanager_worker_id
                    };

                    return &api;
                }

                default:
                    return NULL;
            };

        default:
            return NULL;
    }

}