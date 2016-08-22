//==============================================================================
// !!! REFACTOR THIS SHIT IMPLEMENTATION !!!
//==============================================================================

//==============================================================================
// Includes
//==============================================================================

#include <cetech/taskmanager/types.h>
#include <celib/os/thread.h>
#include <include/SDL2/SDL_cpuinfo.h>

#include "queue_mpmc.h"

//==============================================================================
// Defines
//==============================================================================

#define make_task(i) (task_t){.id = i}

//==============================================================================
// Globals
//==============================================================================

struct task {
    task_work_t task_work;
    void *task_data;
    atomic_int job_count;

    const char *name;

    task_t depend;
    task_t parent;

    enum task_priority priority;
    enum task_affinity affinity;
    int used;
};

static __thread int _worker_id = 0;

static struct G {
    struct task _taskPool[4096];
    int _openTasks[4096];
    struct queue_mpmc _gloalQueue[3];
    struct queue_mpmc *_workersQueue;
    thread_t *_workers;
    int _workers_count;
    spinlock_t openTaskLock;
    atomic_int _openTaskCount;
    int _Run;
} TaskManagerGlobal = {0};

#define _G TaskManagerGlobal

//==============================================================================
// Private
//==============================================================================

static task_t _new_task() {
    for (u32 i = 1; i < 4096; ++i) {
        if (_G._taskPool[i].used) {
            continue;
        }

        _G._taskPool[i].used = 1;

        return make_task(i);
    }

    return task_null;
}

static void _push_task(task_t t) {
    struct queue_mpmc *q;
    switch (_G._taskPool[t.id].affinity) {
        case TASK_AFFINITY_NONE:
            q = &_G._gloalQueue[(int) _G._taskPool[t.id].priority];
            break;

        default:
            q = &_G._workersQueue[(((int) _G._taskPool[t.id].affinity - 1) * 3) + (int) _G._taskPool[t.id].priority];
            break;
    }

    queue_mpmc_push(q, t.id);
}

static int _task_is_done(task_t task) {
    int count = atomic_load(&_G._openTaskCount);

    for (u32 i = 0; i < count; ++i) {
        if (_G._openTasks[i] != task.id) {
            continue;
        }

        return 0;
    }

    return 1;
}

static int _can_work_on(task_t task) {
    return (_G._taskPool[task.id].job_count == 1) &&
           (_G._taskPool[task.id].depend.id == 0 || _task_is_done(_G._taskPool[task.id].depend));
}

static task_t _try_pop(struct queue_mpmc *q) {
    u32 poped_task;

    if (queue_mpmc_pop(q, &poped_task, 0)) {
        if (poped_task != 0) {
            if (_can_work_on(make_task(poped_task))) {
                return make_task(poped_task);
            }

            _push_task(make_task(poped_task));
        }
    }

    return task_null;
}

static void _mark_task_job_done(task_t task) {
    spin_lock(&_G.openTaskLock);

    if (_G._taskPool[task.id].parent.id != 0) {
        atomic_fetch_sub(&_G._taskPool[_G._taskPool[task.id].parent.id].job_count, 1);
    }

    int count = atomic_load(&_G._openTaskCount);
    for (u32 i = 0; i < count; ++i) {
        if (_G._openTasks[i] == task.id) {
            atomic_fetch_sub(&_G._openTaskCount, 1);
            int idx = atomic_load(&_G._openTaskCount);

            _G._openTasks[i] = _G._openTasks[idx];
            _G._openTasks[idx] = 0;

            _G._taskPool[task.id].used = 0;

            break;
        }
    }

    spin_unlock(&_G.openTaskLock);
}

static task_t _task_pop_new_work() {
    task_t popedTask;

    for (u32 i = 0; i < 3; ++i) {
        struct queue_mpmc *q = &_G._workersQueue[_worker_id * 3 + i];

        if (0 == queue_mpmc_size(q)) {
            continue;
        }

        popedTask = _try_pop(q);
        if (popedTask.id != 0) {
            return popedTask;
        }
    }

    for (u32 i = 0; i < 3; ++i) {
        struct queue_mpmc *q = &_G._gloalQueue[i];

        if (0 == queue_mpmc_size(q)) {
            continue;
        }
        popedTask = _try_pop(q);
        if (popedTask.id != 0) {
            return popedTask;
        }
    }

    return task_null;
}

void taskmanager_do_work();

static int _task_worker(void *o) {
    // Wait for run signal 0 -> 1
    while (!_G._Run) {
    }

    _worker_id = (int) o;

    log_debug("task_worker", "Worker %d init", _worker_id);

    while (_G._Run) {
        taskmanager_do_work();
    }

    log_debug("task_worker", "Worker %d shutdown", _worker_id);

    return 1;
}


//==============================================================================
// Interface
//==============================================================================

int taskmanager_init() {
    _G = (struct G) {0};

    uint32_t core_count = SDL_GetCPUCount();

    static const uint32_t main_threads_count = 1 + 1;
    const uint32_t worker_count = core_count - main_threads_count;

    log_info("task", "Core/Main/Worker: %d, %d, %d", core_count, main_threads_count, worker_count);

    _G._workers_count = worker_count;

    _G._workersQueue = CE_ALLOCATE(memsys_main_allocator(), struct queue_mpmc, 3 * (worker_count + 1));
    _G._workers = CE_ALLOCATE(memsys_main_allocator(), thread_t, worker_count);

    for (int i = 0; i < 3; ++i) {
        queue_mpmc_init(&_G._gloalQueue[i], 4096, memsys_main_allocator());
    }

    for (int i = 0; i < 3 * (worker_count + 1); ++i) {
        queue_mpmc_init(&_G._workersQueue[i], 4096, memsys_main_allocator());
    }

    for (int j = 0; j < worker_count; ++j) {
        _G._workers[j] = thread_create((thread_fce_t) _task_worker, "worker", (void *) (j + 1));
    }

    _G._Run = 1;

    return 1;
}

void taskmanager_shutdown() {
    _G._Run = 0;

    int status = 0;

    for (u32 i = 0; i < _G._workers_count; ++i) {
        thread_wait(_G._workers[i], &status);
    }

    for (int i = 0; i < 3; ++i) {
        queue_mpmc_destroy(&_G._gloalQueue[i]);
    }

    for (int i = 0; i < 3 * (_G._workers_count + 1); ++i) {
        queue_mpmc_destroy(&_G._workersQueue[i]);
    }

    CE_DEALLOCATE(memsys_main_allocator(), _G._workersQueue);
    CE_DEALLOCATE(memsys_main_allocator(), _G._workers);

    _G = (struct G) {0};
}

task_t taskmanager_add_begin(const char *name,
                             task_work_t work,
                             void *data,
                             task_t depend,
                             task_t parent,
                             enum task_priority priority,
                             enum task_affinity affinity) {
    task_t task = _new_task();

    _G._taskPool[task.id].name = name;
    _G._taskPool[task.id].priority = priority;
    _G._taskPool[task.id].task_work = work;
    _G._taskPool[task.id].job_count = 2;
    _G._taskPool[task.id].depend = depend;
    _G._taskPool[task.id].parent = parent;
    _G._taskPool[task.id].affinity = affinity;
    _G._taskPool[task.id].task_data = data;

    if (parent.id != 0) {
        atomic_fetch_add(&_G._taskPool[parent.id].job_count, 1);
    }

    {
        int idx = atomic_fetch_add(&_G._openTaskCount, 1);
        _G._openTasks[idx] = task.id;

    }

    return task;
}

static void _null_task(void *d) {
}

task_t taskmanager_add_null(const char *name,
                            task_t depend,
                            task_t parent,
                            enum task_priority priority,
                            enum task_affinity affinity) {

    return taskmanager_add_begin(name, _null_task, NULL, depend, parent, priority, affinity);
}

void taskmanager_add_end(task_t *tasks, size_t count) {
    for (u32 i = 0; i < count; ++i) {
        atomic_fetch_sub(&_G._taskPool[tasks[i].id].job_count, 1);
    }

    for (u32 i = 0; i < count; ++i) {
        _push_task(tasks[i]);
    }
}

void taskmanager_do_work() {
    task_t t = _task_pop_new_work();

    if (t.id == 0) {
        return;
    }

    //log_debug("task", "Work on %d", t.id);
    _G._taskPool[t.id].task_work(_G._taskPool[t.id].task_data);

    _mark_task_job_done(t);
}

void taskmanager_wait(task_t task) {
    while (!_task_is_done(task)) {
        taskmanager_do_work();
    }
}

char taskmanager_worker_id() {
    return (char) _worker_id;
}

int taskmanager_open_task_count() {
    return _G._openTaskCount;
}