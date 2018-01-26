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

int do_work();

//==============================================================================
// Defines
//==============================================================================

#define make_task(i) (task_t){.id = i}

#define MAX_TASK 4096
#define MAX_COUNTERS 4096
#define LOG_WHERE "taskmanager"
#define _G TaskManagerGlobal

//==============================================================================
// Globals
//==============================================================================

struct task {
    void *data;
    void (*task_work)(void *data);
    const char *name;
    atomic_int *counter;
};

typedef struct {
    uint32_t id;
} task_t;

static const task_t task_null = (task_t) {.id = 0};

static struct _G {
    ct_thread_t *_workers[TASK_MAX_WORKERS - 1];

    // TASK
    struct task _task_pool[MAX_TASK];
    atomic_int _task_pool_idx;

    // COUNTERS
    atomic_int _counter_pool_idx;
    atomic_int _counter_pool[MAX_COUNTERS];

    struct task_queue _gloalQueue;

    uint32_t _workers_count;
    int _Run;
} _G;

// Private
static __thread uint8_t _worker_id = 0;

//==============================================================================
//==============================================================================

static task_t _new_task() {
    int idx = atomic_fetch_add(&_G._task_pool_idx, 1);

    if ((idx & (MAX_TASK - 1)) == 0) {
        idx = atomic_fetch_add(&_G._task_pool_idx, 1);
    }

    return make_task((uint32_t) (idx & (MAX_TASK - 1)));
}

static uint32_t _new_counter_task(uint32_t value) {
    int idx = atomic_fetch_add(&_G._counter_pool_idx, 1);

    if ((idx & (MAX_COUNTERS - 1)) == 0) {
        idx = atomic_fetch_add(&_G._counter_pool_idx, 1);
    }

    uint32_t ret_idx = (uint32_t) (idx & (MAX_TASK - 1));

    atomic_int *counter = &_G._counter_pool[ret_idx];
    atomic_init(counter, value);

    return ret_idx;
}

static void _push_task(task_t t) {
    struct task_queue *q;
    q = &_G._gloalQueue;

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

static task_t _task_pop_new_work() {
    task_t popedTask;
    struct task_queue *qg = &_G._gloalQueue;

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
        if (!do_work()) {
            ct_thread_a0.yield();
        }
    }

    ct_log_a0.debug("task_worker", "Worker %d shutdown", _worker_id);
    return 1;
}



//==============================================================================
// Api
//==============================================================================

void add(struct ct_task_item *items,
         uint32_t count,
         struct ct_task_counter_t **counter) {
    uint32_t new_counter = _new_counter_task(count);

    atomic_int *cnt = &_G._counter_pool[new_counter];
    *counter = (struct ct_task_counter_t *) &_G._counter_pool[new_counter];

    for (uint32_t i = 0; i < count; ++i) {
        task_t task = _new_task();
        _G._task_pool[task.id] = (struct task) {
                .name = items[i].name,
                .task_work = items[i].work,
                .counter = cnt
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

    struct task *task = &_G._task_pool[t.id];

    task->task_work(_G._task_pool[t.id].data);
    atomic_fetch_sub(task->counter, 1);

    return 1;
}

void wait_atomic(struct ct_task_counter_t *signal,
                 int32_t value) {
    while (atomic_load_explicit((atomic_int *) signal, memory_order_acquire) !=
           value) {
        do_work();
    }
}

char worker_id() {
    return _worker_id;
}

int worker_count() {
    return _G._workers_count;
}

static void _init_api(struct ct_api_a0 *api) {
    static struct ct_task_a0 _api = {
            .worker_id = worker_id,
            .worker_count = worker_count,
            .add = add,
            .do_work = do_work,
            .wait_for_counter = wait_atomic
    };

    api->register_api("ct_task_a0", &_api);
}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_thread_a0);
    CETECH_GET_API(api, ct_log_a0);
    CETECH_GET_API(api, ct_cpu_a0);

    ct_log_a0.set_wid_clb(worker_id);

    _G = (struct _G) {};

    int core_count = ct_cpu_a0.count();

    static const uint32_t main_threads_count = 1 + 1/* Renderer */;
    const uint32_t worker_count = core_count - main_threads_count;

    ct_log_a0.info("task", "Core/Main/Worker: %d, %d, %d", core_count,
                   main_threads_count, worker_count);

    _G._workers_count = worker_count;

    queue_task_init(&_G._gloalQueue, MAX_TASK,
                    ct_memory_a0.main_allocator());

    for (uint32_t j = 0; j < worker_count; ++j) {
        _G._workers[j] = ct_thread_a0.create(
                (ct_thread_fce_t) _task_worker,
                "worker",
                (void *) ((intptr_t) (j + 1)));
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

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        task,
        {
            CT_UNUSED(api);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)
