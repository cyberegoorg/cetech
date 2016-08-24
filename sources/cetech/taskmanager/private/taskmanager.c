//==============================================================================
// !!! REFACTOR THIS SHIT IMPLEMENTATION !!!
//==============================================================================

//==============================================================================
// Includes
//==============================================================================

#include "cetech/taskmanager/types.h"
#include "celib/machine/thread.h"
#include <include/SDL2/SDL_cpuinfo.h>

#include "queue_task.h"

//==============================================================================
// Defines
//==============================================================================

#define make_task(i) (task_t){.id = i}

#define MAX_TASK 4096
#define MAX_CONTINUES 32
#define MAX_TASK_DATA_SIZE 64
#define LOG_WHERE "taskmanager"

//==============================================================================
// Globals
//==============================================================================

struct task {
    task_t continues[MAX_CONTINUES];
    u8 data[MAX_TASK_DATA_SIZE];
    task_work_t task_work;
    const char *name;
    atomic_int job_count;
    atomic_int continues_count;

    task_t parent;
    task_t depend_on;

    enum task_priority priority;
    enum task_affinity affinity;
};

static __thread char _worker_id = 0;

static struct G {
    struct task _taskPool[MAX_TASK];
    char _openTasks[MAX_TASK];
    struct queue_task _gloalQueue[3];
    struct queue_task _workersQueue[MAX_WORKERS * 3];
    thread_t _workers[MAX_WORKERS - 1];
    atomic_int _task_pool_idx;
    int _workers_count;
    int _Run;
} TaskManagerGlobal = {0};

#define _G TaskManagerGlobal

//==============================================================================
// Private
//==============================================================================

static task_t _new_task() {
    int idx = atomic_fetch_add(&_G._task_pool_idx, 1);

    if((idx & (MAX_TASK - 1)) == 0) {
        idx = atomic_fetch_add(&_G._task_pool_idx, 1);
    }

    return make_task(idx & (MAX_TASK - 1));
}

static void _push_task(task_t t) {
    CE_ASSERT("", t.id != 0);

    struct queue_task *q;
    switch (_G._taskPool[t.id].affinity) {
        case TASK_AFFINITY_NONE:
            q = &_G._gloalQueue[(int) _G._taskPool[t.id].priority];
            break;

        default:
            q = &_G._workersQueue[(((int) _G._taskPool[t.id].affinity - 1) * 3) + (int) _G._taskPool[t.id].priority];
            break;
    }

    queue_task_push(q, t.id);
}

static int _task_is_done(task_t task) {
    return !_G._openTasks[task.id];
}

static task_t _try_pop(struct queue_task *q) {
    u32 poped_task;

    if (queue_task_pop(q, &poped_task, 0)) {
        if (poped_task != 0) {
            return make_task(poped_task);
        }
    }

    return task_null;
}

static void _mark_task_job_done(task_t task) {
    _G._openTasks[task.id] = 0;

    u32 parent_idx = _G._taskPool[task.id].parent.id;
    if (parent_idx != 0) {
        int count = atomic_fetch_sub(&_G._taskPool[parent_idx].job_count, 1);
        if(count == 2) {
            _push_task(make_task(parent_idx));
        }
    }

    for (int i = 0; i < _G._taskPool[task.id].continues_count; ++i) {
        _push_task(_G._taskPool[task.id].continues[i]);
    }
    
    _G._taskPool[task.id].job_count = 0;
}

static task_t _task_pop_new_work() {
    task_t popedTask;

    for (u32 i = 0; i < 3; ++i) {
        struct queue_task *qw = &_G._workersQueue[_worker_id * 3 + i];
        struct queue_task *qg = &_G._gloalQueue[i];

        popedTask = _try_pop(qw);
        if (popedTask.id != 0) {
            return popedTask;
        }

        popedTask = _try_pop(qg);
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
    log_debug(LOG_WHERE, "Init (global size: %lu, task size: %lu)", sizeof(struct G), sizeof(struct task));

    _G = (struct G) {0};
    memset(_G._openTasks, 0, sizeof(char) * 1024);

    uint32_t core_count = SDL_GetCPUCount();

    static const uint32_t main_threads_count = 1 + 1;
    const uint32_t worker_count = core_count - main_threads_count;

    log_info("task", "Core/Main/Worker: %d, %d, %d", core_count, main_threads_count, worker_count);

    _G._workers_count = worker_count;

    for (int i = 0; i < 3; ++i) {
        queue_task_init(&_G._gloalQueue[i], MAX_TASK, memsys_main_allocator());
    }

    for (int i = 0; i < 3 * (worker_count + 1); ++i) {
        queue_task_init(&_G._workersQueue[i], MAX_TASK, memsys_main_allocator());
    }

    for (int j = 0; j < worker_count; ++j) {
        _G._workers[j] = thread_create((thread_fce_t) _task_worker, "worker", (void *) ((intptr_t)(j + 1)));
    }

    _G._Run = 1;

    return 1;
}

void taskmanager_shutdown() {
    _G._Run = 0;

    int status = 0;

    for (u32 i = 0; i < _G._workers_count; ++i) {
        //thread_kill(_G._workers[i]);
        thread_wait(_G._workers[i], &status);
    }

    for (int i = 0; i < 3; ++i) {
        queue_task_destroy(&_G._gloalQueue[i]);
    }

    for (int i = 0; i < 3 * (_G._workers_count + 1); ++i) {
        queue_task_destroy(&_G._workersQueue[i]);
    }

    _G = (struct G) {0};
}

task_t taskmanager_add_begin(const char *name,
                             task_work_t work,
                             void *data,
                             char data_len,
                             task_t depend,
                             task_t parent,
                             enum task_priority priority,
                             enum task_affinity affinity) {
    task_t task = _new_task();

    _G._openTasks[task.id] = 1;

    _G._taskPool[task.id].name = name;
    _G._taskPool[task.id].priority = priority;
    _G._taskPool[task.id].task_work = work;
    _G._taskPool[task.id].job_count = 1;

    _G._taskPool[task.id].depend_on = depend;
    _G._taskPool[task.id].continues_count = 0;
    _G._taskPool[task.id].parent = parent;
    _G._taskPool[task.id].affinity = affinity;

    memcpy(_G._taskPool[task.id].data, data, (size_t) data_len);

    if (parent.id != 0) {
        atomic_fetch_add(&_G._taskPool[parent.id].job_count, 1);
    }

    if(depend.id != 0) {
        int idx = atomic_fetch_add(&_G._taskPool[depend.id].continues_count, 1);

        _G._taskPool[depend.id].continues[idx] = task;
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

    return taskmanager_add_begin(name, _null_task, NULL, 0, depend, parent, priority, affinity);
}

void taskmanager_add_end(const task_t *tasks, size_t count) {
    for (u32 i = 0; i < count; ++i) {
        if(_G._taskPool[tasks[i].id].depend_on.id == 0) {
            if (atomic_load(&_G._taskPool[tasks[i].id].job_count) == 1) {
                _push_task(tasks[i]);
            }
        }
    }
}

void taskmanager_do_work() {
    task_t t = _task_pop_new_work();

    if (t.id == 0) {
        return;
    }

    _G._taskPool[t.id].task_work(_G._taskPool[t.id].data);

    _mark_task_job_done(t);
}

void taskmanager_wait(task_t task) {
    while (!_task_is_done(task)) {
        taskmanager_do_work();
    }
}

char taskmanager_worker_id() {
    return _worker_id;
}
