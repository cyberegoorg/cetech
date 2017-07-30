//==============================================================================
// Includes
//==============================================================================


#include <cetech/core/api/api_system.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/os/thread.h>
#include <cetech/core/os/cpu.h>
#include <cetech/core/log/log.h>
#include <cetech/core/task/task.h>
#include <cetech/core/module/module.h>

#include "task_queue.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_thread_a0);
CETECH_DECL_API(ct_cpu_a0);
CETECH_DECL_API(ct_log_a0);

namespace taskmanager {
    int do_work();
}
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

    void (*task_work)(void *data);

    const char *name;
    ct_task_affinity affinity;
};

static __thread uint8_t _worker_id = 0;

#define _G TaskManagerGlobal
static struct G {
    struct task _task_pool[MAX_TASK];
    struct task_queue _workers_queue[TASK_MAX_WORKERS];
    ct_thread_t *_workers[TASK_MAX_WORKERS - 1];
    struct task_queue _gloalQueue;
    atomic_int _task_pool_idx;
    uint32_t _workers_count;
    int _Run;
} TaskManagerGlobal = {};


//==============================================================================
// Private
//==============================================================================

typedef struct {
    uint32_t id;
} task_t;

static const task_t task_null = (task_t) {.id = 0};

static task_t _new_task() {
    int idx = atomic_fetch_add(&_G._task_pool_idx, 1);

    if ((idx & (MAX_TASK - 1)) == 0) {
        idx = atomic_fetch_add(&_G._task_pool_idx, 1);
    }

    return make_task((uint32_t) (idx & (MAX_TASK - 1)));
}

static void _push_task(task_t t) {
    CETECH_ASSERT("", t.id != 0);

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
    uint32_t poped_task;

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
    CEL_UNUSED(task);
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


static int _task_worker(void *o) {
    // Wait for run signal 0 -> 1
    while (!_G._Run) {
    }

    _worker_id = (char) (uint64_t) o;

    ct_log_a0.debug("task_worker", "Worker %d init", _worker_id);

    while (_G._Run) {
        if (!taskmanager::do_work()) {
            ct_thread_a0.yield();
        }
    }

    ct_log_a0.debug("task_worker", "Worker %d shutdown", _worker_id);
    return 1;
}



//==============================================================================
// Api
//==============================================================================

namespace taskmanager {
    void add(ct_task_item *items,
             uint32_t count) {
        for (uint32_t i = 0; i < count; ++i) {
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

    int do_work() {
        task_t t = _task_pop_new_work();

        if (t.id == 0) {
            return 0;
        }

        _G._task_pool[t.id].task_work(_G._task_pool[t.id].data);

        _mark_task_job_done(t);

        return 1;
    }

    void wait_atomic(atomic_int *signal,
                     int32_t value) {
        while (atomic_load_explicit(signal, memory_order_acquire) == value) {
            do_work();
        }
    }

    char worker_id() {
        return _worker_id;
    }

    int worker_count() {
        return _G._workers_count;
    }
}

namespace taskmanager_module {
    static void _init_api(ct_api_a0 *api) {
        static ct_task_a0 _api = {
                .worker_id = taskmanager::worker_id,
                .worker_count = taskmanager::worker_count,
                .add = taskmanager::add,
                .do_work = taskmanager::do_work,
                .wait_atomic = taskmanager::wait_atomic
        };

        api->register_api("ct_task_a0", &_api);
    }

    static void _init(ct_api_a0 *api) {
        _init_api(api);

        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_thread_a0);
        CETECH_GET_API(api, ct_log_a0);
        CETECH_GET_API(api, ct_cpu_a0);

        ct_log_a0.set_wid_clb(taskmanager::worker_id);

        _G = {};

        int core_count = ct_cpu_a0.count();

        static const uint32_t main_threads_count = 1;
        const uint32_t worker_count = core_count - main_threads_count;

        ct_log_a0.info("task", "Core/Main/Worker: %d, %d, %d", core_count,
                       main_threads_count, worker_count);

        _G._workers_count = worker_count;

        queue_task_init(&_G._gloalQueue, MAX_TASK,
                        ct_memory_a0.main_allocator());

        for (uint32_t i = 0; i < worker_count + 1; ++i) {
            queue_task_init(&_G._workers_queue[i], MAX_TASK,
                            ct_memory_a0.main_allocator());
        }

        for (uint32_t j = 0; j < worker_count; ++j) {
            _G._workers[j] = ct_thread_a0.create(
                    (ct_thread_fce_t) _task_worker,
                    "worker",
                    (void *) ((intptr_t) (j +
                                          1)));
        }

        _G._Run = 1;
    }

    static void _shutdown() {
        _G._Run = 0;
        int status = 0;

        for (uint32_t i = 0; i < _G._workers_count; ++i) {
            ct_thread_a0.wait(_G._workers[i], &status);
        }

        queue_task_destroy(&_G._gloalQueue);

        for (uint32_t i = 0; i < _G._workers_count + 1; ++i) {
            queue_task_destroy(&_G._workers_queue[i]);
        }

        _G = {};
    }
}


CETECH_MODULE_DEF(
        task,
        {

        },
        {
            taskmanager_module::_init(api);
        },
        {
            CEL_UNUSED(api);
            taskmanager_module::_shutdown();
        }
)
