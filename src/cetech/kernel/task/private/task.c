//==============================================================================
// Includes
//==============================================================================


#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/os/thread.h>
#include <cetech/kernel/os/cpu.h>
#include <cetech/kernel/log/log.h>
#include <cetech/kernel/task/task.h>
#include <cetech/kernel/module/module.h>

#include "queue_mpmc.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_thread_a0);
CETECH_DECL_API(ct_cpu_a0);
CETECH_DECL_API(ct_log_a0);

//==============================================================================
// Defines
//==============================================================================

#define make_task(i) (task_id_t){.id = i}

#define MAX_TASK 4096
#define MAX_COUNTERS 4096
#define LOG_WHERE "taskmanager"
#define _G TaskManagerGlobal

//==============================================================================
// Globals
//==============================================================================

struct task_t {
    void *data;

    void (*task_work)(void *data);

    const char *name;
    uint32_t counter;
};

typedef struct {
    uint32_t id;
} task_id_t;

static const task_id_t task_null = (task_id_t) {.id = 0};

static struct _G {
    ct_thread_t *workers[TASK_MAX_WORKERS - 1];

    // TASK
    struct task_t task_pool[MAX_TASK];
    atomic_int task_pool_idx;
    struct queue_mpmc free_task;

    // COUNTERS
    atomic_int counter_pool_idx;
    atomic_int counter_pool[MAX_COUNTERS];
    struct queue_mpmc free_counter;

    uint32_t workers_count;

    struct queue_mpmc job_queue;
    int is_running;
} _G;

// Private
static __thread uint8_t _worker_id = 0;

//==============================================================================
//==============================================================================

static task_id_t _new_task() {
    uint32_t idx;
    queue_task_pop(&_G.free_task, &idx, UINT32_MAX);

    if (idx == UINT32_MAX) {
        idx = atomic_fetch_add(&_G.task_pool_idx, 1);
    }

    return make_task(idx);
}

static uint32_t _new_counter_task(uint32_t value) {
    uint32_t idx;
    queue_task_pop(&_G.free_counter, &idx, UINT32_MAX);

    if (idx == UINT32_MAX) {
        idx = atomic_fetch_add(&_G.counter_pool_idx, 1);
    }

    atomic_int *counter = &_G.counter_pool[idx];
    atomic_init(counter, value);

    return idx;
}

static void _push_task(task_id_t t) {
    struct queue_mpmc *q;
    q = &_G.job_queue;

    queue_task_push(q, t.id);
}


static task_id_t _try_pop(struct queue_mpmc *q) {
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

static task_id_t _task_pop_new_work() {
    task_id_t pop_task;
    struct queue_mpmc *qg = &_G.job_queue;

    pop_task = _try_pop(qg);
    if (pop_task.id != 0) {
        return pop_task;
    }

    return task_null;
}


int do_work() {
    task_id_t t = _task_pop_new_work();

    if (t.id == 0) {
        return 0;
    }

    struct task_t *task = &_G.task_pool[t.id];

    task->task_work(_G.task_pool[t.id].data);

    atomic_fetch_sub(&_G.counter_pool[task->counter], 1);
    queue_task_push(&_G.free_task, t.id);

    return 1;
}

static int _task_worker(void *o) {
    // Wait for run signal 0 -> 1
    while (!_G.is_running) {
    }

    _worker_id = (char) (uint64_t) o;

    ct_log_a0.debug("task_worker", "Worker %d init", _worker_id);

    while (_G.is_running) {
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

    *counter = (struct ct_task_counter_t *) &_G.counter_pool[new_counter];

    for (uint32_t i = 0; i < count; ++i) {
        task_id_t task = _new_task();
        _G.task_pool[task.id] = (struct task_t) {
                .name = items[i].name,
                .task_work = items[i].work,
                .counter = new_counter
        };

        _G.task_pool[task.id].data = items[i].data;

        _push_task(task);
    }
}


void wait_atomic(struct ct_task_counter_t *signal,
                 int32_t value) {
    while (atomic_load_explicit((atomic_int *) signal, memory_order_acquire) !=
           value) {
        do_work();
    }

    uint32_t counter_idx = ((atomic_int *) signal) - _G.counter_pool;
    queue_task_push(&_G.free_counter, counter_idx);
}

char worker_id() {
    return _worker_id;
}

int worker_count() {
    return _G.workers_count;
}

static struct ct_task_a0 _task_api = {
        .worker_id = worker_id,
        .worker_count = worker_count,
        .add = add,
        .wait_for_counter = wait_atomic
};

static void _init(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_thread_a0);
    CETECH_GET_API(api, ct_log_a0);
    CETECH_GET_API(api, ct_cpu_a0);

    api->register_api("ct_task_a0", &_task_api);
    ct_log_a0.set_wid_clb(worker_id);

    _G = (struct _G) {};

    int core_count = ct_cpu_a0.count();

    static const uint32_t main_threads_count = 1 + 1/* Renderer */;
    const uint32_t worker_count = core_count - main_threads_count;

    ct_log_a0.info("task", "Core/Main/Worker: %d, %d, %d", core_count,
                   main_threads_count, worker_count);

    _G.workers_count = worker_count;

    queue_task_init(&_G.job_queue, MAX_TASK, ct_memory_a0.main_allocator());
    queue_task_init(&_G.free_task, MAX_TASK, ct_memory_a0.main_allocator());
    queue_task_init(&_G.free_counter, MAX_TASK, ct_memory_a0.main_allocator());

    atomic_init(&_G.counter_pool_idx, 1);
    atomic_init(&_G.task_pool_idx, 1);

    for (uint32_t j = 0; j < worker_count; ++j) {
        _G.workers[j] = ct_thread_a0.create(_task_worker, "worker",
                                             (void *) ((intptr_t) (j + 1)));
    }

    _G.is_running = 1;
}

static void _shutdown() {
    _G.is_running = 0;
    int status = 0;

    for (uint32_t i = 0; i < _G.workers_count; ++i) {
        ct_thread_a0.wait(_G.workers[i], &status);
    }

    queue_task_destroy(&_G.job_queue);
    queue_task_destroy(&_G.free_task);
    queue_task_destroy(&_G.free_counter);

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
