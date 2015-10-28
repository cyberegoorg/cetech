#include "cetech/task_manager/task_manager.h"

#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"
#include "celib/container/queue.inl.h"

#include "cetech/thread/thread.h"
#include "cetech/application/application.h"
#include "cetech/log_system/log_system.h"

#include <unistd.h>

#define TASK_INITIALIZER { { 0 }, { 0 }, { 0 }, 0, 0, WorkerAffinity::NONE, { 0, 0 } }

//TODO: REWRITE !!! #62
namespace cetech {
    namespace {
        using namespace task_manager;

        /*!
         * Task worker callback.
         */
        struct TaskWorkCallback {
            TaskWorkFce_t fce;    //!< Worker fce.
            void* data;           //!< Worker data.
        };

        struct Task {
            TaskID id;                  //!< Task id
            TaskID depend;              //!< Task depend
            TaskID parent;              //!< Task parent

            uint32_t priority;                    //!< Task priority
            uint32_t job_count;                   //!< Task child active job
            WorkerAffinity::Enum worker_affinity; //!< Worker affinity
            TaskWorkCallback clb;                 //!< Callback
        };

        enum {
            MAX_TASK = 4096
        };


        struct TaskQueue {
            enum {
                MASK = MAX_TASK - 1u
            };

            Spinlock _lock;
            uint32_t _top;
            uint32_t _bottom;

            uint32_t _data[MAX_TASK];

            TaskQueue() : _top(0), _bottom(0) {
                memset(_data, 0, sizeof(uint32_t) * MAX_TASK);
            }
        };

        namespace taskqueue {
            void push(TaskQueue& q, uint32_t task) {
                thread::spin_lock(q._lock);

                q._data[q._bottom & TaskQueue ::MASK] = task + 1;
                ++q._bottom;

                thread::spin_unlock(q._lock);
            }

            uint32_t pop(TaskQueue& q) {
                thread::spin_lock(q._lock);

                const int jobCount = q._bottom - q._top;
                if (jobCount <= 0) {
                    thread::spin_unlock(q._lock);
                    return 0;
                }

                uint32_t task = q._data[q._top & TaskQueue ::MASK];
                ++q._top;

                thread::spin_unlock(q._lock);
                return task;
            }
        }

        static thread_local uint32_t _worker_id = 0; // Worker id (thanks thread_local)
        struct TaskManagerData {
            Spinlock _lock;

            uint32_t _open_task_count;        //!< Open task count
            uint32_t _open_task[MAX_TASK];    //!< Open task

            Task _task_pool[MAX_TASK];        //!< Task pool

            Array < Thread > _workers;

            TaskQueue _global_queue;

            TaskQueue _workers_queue[3]; // TODO: dynamic

            Allocator& _allocator;

            struct {
                char run : 1;
            } flags;

            TaskManagerData(Allocator & allocator) : _open_task_count(0),
                                                     _workers(allocator),
                                                     _allocator(allocator) {

                memset(_open_task, 0, sizeof(uint32_t) * MAX_TASK);
                memset(_task_pool, 0, sizeof(Task) * MAX_TASK);
            }

            ~TaskManagerData() {
                for (uint32_t i = 0; i < array::size(_workers); ++i) {
                    log::debug("task", "Killing worker%u.", i);
                    thread::kill(_workers[i]);
                }
            };
        };

        struct Globals {
            static const int MEMORY = sizeof(TaskManagerData);
            char buffer[MEMORY];

            TaskManagerData* data;

            Globals() : data(0) {}
        } _globals;

        uint32_t _core_count() {
#if defined(CETECH_SDL2)
            return SDL_GetCPUCount();
#endif
        }

        static int task_worker(void* data) {
            // Wait for run signal 0 -> 1
            while (_globals.data->flags.run == 0) {}

            _worker_id = (uint64_t)data; // TODO: (uint64_t)?? !!!

            log::info("task_worker", "Worker init %d", _worker_id);

            while (_globals.data->flags.run) {
                task_manager::do_work();
            }

            log::info("task_worker", "Worker shutdown");
            return 0;
        }


        /*! NOP task.
         */
        static void _task_nop(void*) {}

        /*! Get new task idx from pool.
         * \return Task idx.
         */
        uint32_t _new_task() {
            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;

            for (uint32_t i = 1; i < MAX_TASK; ++i) {
                if (tm._task_pool[i].id.i != 0) {
                    continue;
                }

                return i;
            }

            CE_ASSERT_MSG(false, "Pool overflow");
            abort();
        }

        /*! Find task in pool
         * \param id Task id.
         * \return Task ptr.
         */
        Task* get_task(TaskID id) {
            return &_globals.data->_task_pool[id.i];
        }

        void push_task(Task* task) {
            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;

            WorkerAffinity::Enum aff = task->worker_affinity;

            switch (aff) {
            case WorkerAffinity::NONE:
                taskqueue::push(tm._global_queue, task->id.i);
                break;

            default:
                taskqueue::push(tm._workers_queue[aff - 1], task->id.i);
                break;
            }
        }

        /*! Mark job as done.
         * \param id Task id.
         */
        void mark_task_job_done(TaskID id) {
            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;

            thread::spin_lock(tm._lock);

            for (uint32_t i = 0; i < tm._open_task_count; ++i) {
                Task* t = &tm._task_pool[tm._open_task[i]];

                if (t->id.i != id.i) {

                    if (t->depend.i == id.i) {
                        push_task(get_task(t->depend));
                    }

                    continue;
                }

                t->id.i = 0;

                if (t->parent.i != 0) {
                    Task* parent_task = get_task(t->parent);
                    --parent_task->job_count;

                    if (parent_task->job_count == 1) {
                        push_task(parent_task);
                    }
                }

                tm._open_task[i] = tm._open_task[tm._open_task_count - 1];
                --tm._open_task_count;
                continue;
            }

            thread::spin_unlock(tm._lock);
        }

        int8_t task_is_null(TaskID id) {
            return id.i == 0;
        }

        bool task_is_done(TaskID id) {
            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;

            for (uint32_t i = 0; i < tm._open_task_count; ++i) {
                if (tm._task_pool[tm._open_task[i]].id.i != id.i) {
                    continue;
                }

                return false;
            }

            return true;
        }

        Task task_pop_new_work() {
            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;


            //const uint32_t id = _worker_id + 1;
            uint32_t poped_task;

            TaskQueue& q = tm._workers_queue[_worker_id];
            poped_task = taskqueue::pop(q);
            if (poped_task != 0) {
                Task t = tm._task_pool[poped_task - 1];
                return t;
            }

            poped_task = taskqueue::pop(tm._global_queue);
            if (poped_task != 0) {
                Task t = tm._task_pool[poped_task - 1];
                return t;
            }

            return (Task)TASK_INITIALIZER;
        }

    }

    namespace task_manager {
        uint32_t get_worker_id() {
            return _worker_id;
        }

        TaskID add_begin(const TaskWorkFce_t fce,
                         void* data,
                         const uint32_t priority,
                         const TaskID depend,
                         const TaskID parent,
                         const WorkerAffinity::Enum worker_affinity) {

            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;

            TaskWorkCallback callback = { fce, data };

            uint32_t task = _new_task();

            Task t;
            t.id = { task },
            t.priority = priority,
            t.clb = callback,
            t.job_count = 2,
            t.depend = depend,
            t.parent = parent,
            t.worker_affinity = worker_affinity;

            tm._task_pool[task] = t;

            if (parent.i != 0) {
                Task* t = get_task(parent);
                ++t->job_count;
            }

            tm._open_task[tm._open_task_count] = task;
            ++tm._open_task_count;

            return (TaskID) {
                       task
            };

        }

        TaskID add_empty_begin(const uint32_t priority,
                               const TaskID depend,
                               const TaskID parent,
                               const WorkerAffinity::Enum worker_affinity) {
            return add_begin(_task_nop, 0, priority, depend, parent, worker_affinity);
        }

        void add_end(const TaskID* tasks, const uint32_t count) {
            for (uint32_t i = 0; i < count; ++i) {
                Task* t = get_task(tasks[i]);

                --t->job_count;

                if (t->job_count == 1) {
                    push_task(t);
                }
            }
        }

        void do_work() {
            Task t = task_pop_new_work();

            if (t.id.i == 0) {
                usleep(0);
                return;
            }

            CE_ASSERT(t.clb.fce != NULL);

            t.clb.fce(t.clb.data);

            mark_task_job_done(t.id);

        }

        void wait(const TaskID id) {
            while (_globals.data->flags.run && !task_is_done(id)) {
                do_work();
            }
        }

        void spawn_workers() {
            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;

            uint32_t core_count = _core_count();

            static const uint32_t main_threads_count = 1 + 1;
            const uint32_t worker_count = core_count - main_threads_count;

            log::info("task", "Core count: %u", core_count);
            log::info("task", "Main thread count: %u", main_threads_count);
            log::info("task", "Worker count: %u", worker_count);

            if (worker_count > 0) {
                for (uint32_t i = 0; i < worker_count; ++i) {
                    log::debug("task", "Creating worker %u.", i);
                    array::push_back(tm._workers, thread::create_thread(task_worker, "Worker", (void*)(i + 1))); // TODO: (void*)?? !!!
                }
            }

            tm.flags.run = 1;
        }

        void stop() {
            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;

            tm.flags.run = 0;
        }

    }

    namespace task_manager_globals {
        void init() {
            log::info("task_manager_globals", "Init");
            char* p = _globals.buffer;
            _globals.data = new(p) TaskManagerData(memory_globals::default_allocator());

            spawn_workers();
        }

        void shutdown() {
            log::info("task_manager_globals", "Shutdown");

            _globals.data->~TaskManagerData();
            _globals = Globals();
        }
    }

}