#include "cetech/task_manager/task_manager.h"

#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"
#include "celib/container/queue.inl.h"

#include "cetech/thread/thread.h"
#include "cetech/application/application.h"
#include "cetech/log_system/log_system.h"

#include <unistd.h>

#define TASK_INITIALIZER { { 0 }, { 0 }, { 0 }, 0, 0, { 0, 0 } }

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

            uint32_t priority;         //!< Task priority
            uint32_t job_count;        //!< Task child active job
            TaskWorkCallback clb;      //!< Callback
        };

        enum {
            MAX_TASK = 512
        };


        template < int SIZE >
        struct TaskQueue {
            enum {
                MASK = SIZE - 1u
            };

            Spinlock _lock;
            uint32_t _top;
            uint32_t _bottom;

            uint32_t _data[SIZE];

            TaskQueue() : _top(0), _bottom(0) {
                //SDL_AtomicSet(&_top, 0);
                //SDL_AtomicSet(&_bottom, 0);
                memset(_data, 0, sizeof(uint32_t) * SIZE);
            }
        };

        namespace taskqueue {
            template < int SIZE >
            void push(TaskQueue < SIZE >& q, uint32_t task) {
                thread::spin_lock(q._lock);

                q._data[q._bottom & TaskQueue < SIZE > ::MASK] = task + 1;
                ++q._bottom;

                thread::spin_unlock(q._lock);
            }

            template < int SIZE >
            uint32_t pop(TaskQueue < SIZE >& q) {
                thread::spin_lock(q._lock);

                const int jobCount = q._bottom - q._top;
                if (jobCount <= 0) {
                    thread::spin_unlock(q._lock);
                    return 0;
                }

                uint32_t task = q._data[q._top & TaskQueue < SIZE > ::MASK];
                ++q._top;

                thread::spin_unlock(q._lock);
                return task;
            }
        }

        struct TaskManagerData {
            Spinlock _lock;

            uint32_t _last_id;                //!< Last id
            uint32_t _open_task_count;        //!< Open task count

            uint32_t _open_task[MAX_TASK];    //!< Open task
            Task _task_pool[MAX_TASK];        //!< Task pool

            Array < Thread > _workers;
            TaskQueue < MAX_TASK > _tqueue;

            struct {
                char run : 1;
            } flags;

            TaskManagerData(Allocator & allocator) : _last_id(0),
                                                     _open_task_count(0),
                                                     _workers(allocator) {

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
            while (_globals.data->flags.run == 0) {}

            log::info("task_worker", "Worker init");

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
         * \retval NULL Task not found.
         */
        CE_INLINE Task* get_task(TaskID id) {
            return &_globals.data->_task_pool[id.i];
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
                    continue;
                }

                t->id.i = 0;

                if (t->parent.i != 0) {
                    Task* parent_task = get_task(t->parent);
                    --parent_task->job_count;
                }

                tm._open_task[i] = tm._open_task[tm._open_task_count - 1];
                --tm._open_task_count;
                break;
            }

            thread::spin_unlock(tm._lock);
        }

        int8_t task_is_null(TaskID id) {
            return id.i == 0;
        }

        uint32_t task_get_worker_id() {
            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;

            uint32_t id = thread::id();

            for (uint8_t i = 0; i < array::size(tm._workers); ++i) {
                if (thread::get_id(tm._workers[i]) != id) {
                    continue;
                }

                return i + 1;
            }

            return 0;
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

begin:
            uint32_t poped_task = taskqueue::pop(tm._tqueue);
            if (poped_task == 0) {
                return (Task)TASK_INITIALIZER;
            }

            Task t = tm._task_pool[poped_task - 1];

            if (t.job_count != 1) {
                taskqueue::push(tm._tqueue, poped_task - 1);
                goto begin;
                return (Task)TASK_INITIALIZER;
            }

            if ((t.depend.i > 0) && !task_is_done(t.depend)) {
                taskqueue::push(tm._tqueue, poped_task - 1);
                goto begin;
                return (Task)TASK_INITIALIZER;
            }

            return t;
        }

    }

    namespace task_manager {
        TaskID add_begin(const TaskWorkFce_t fce,
                         void* data,
                         const uint32_t priority,
                         const TaskID depend,
                         const TaskID parent) {

            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;

            TaskWorkCallback callback = { fce, data };

            const uint32_t id = ++tm._last_id;

            uint32_t task = _new_task();

            Task t;
            t.id = { task },
            t.priority = priority,
            t.clb = callback,
            t.job_count = 2,
            t.depend = depend,
            t.parent = parent,

            tm._task_pool[task] = t;

            if (parent.i != 0) {
                Task* t = get_task(parent);
                ++t->job_count;
            }

            tm._open_task[tm._open_task_count] = task;
            ++tm._open_task_count;

            taskqueue::push(tm._tqueue, task);

            return (TaskID) {
                       task
            };

        }

        TaskID add_empty_begin(const uint32_t priority, const TaskID depend, const TaskID parent) {
            return add_begin(_task_nop, 0, priority, depend, parent);
        }

        void add_end(const TaskID* tasks, const uint32_t count) {
            for (uint32_t i = 0; i < count; ++i) {
                Task* t = get_task(tasks[i]);

                --t->job_count;
            }
        }

        void do_work() {
            Task t = task_pop_new_work();

            if (t.id.i == 0) {
                return;
            }

            CE_ASSERT(t.clb.fce != NULL);

            t.clb.fce(t.clb.data);

            mark_task_job_done(t.id);

        }

        void wait(const TaskID id) {
            while (!task_is_done(id)) {
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
                    array::push_back(tm._workers, thread::create_thread(task_worker, "Worker", 0));
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