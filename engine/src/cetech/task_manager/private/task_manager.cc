#include "cetech/task_manager/task_manager.h"

#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"
#include "cetech/platform/cpu.h"
#include "cetech/platform/thread.h"

#include "cetech/application/application.h"
#include "cetech/log_system/log_system.h"

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

        struct TaskManagerData {
            Spinlock _lock;               //!< Spinlock

            uint32_t _last_id;                //!< Last id
            uint32_t _task_count;             //!< Task count
            uint32_t _open_task_count;        //!< Open task count

            uint32_t _task_queue[MAX_TASK];   //!< Task queue
            uint32_t _open_task[MAX_TASK];    //!< Open task
            Task _task_pool[MAX_TASK];        //!< Task pool

            Array < Thread > _workers;

            struct {
                char run : 1;
            } flags;

            TaskManagerData(Allocator & allocator) : _last_id(0), _task_count(0), _open_task_count(0), _workers(
                                                         allocator) {
                memset(_task_queue, 0, sizeof(uint32_t) * MAX_TASK);
                memset(_open_task, 0, sizeof(uint32_t) * MAX_TASK);
                memset(_task_pool, 0, sizeof(Task) * MAX_TASK);
            }

            ~TaskManagerData() {
                for (uint32_t i = 0; i < array::size(_workers); ++i) {
                    log_globals::log().debug("task", "Killing worker%u.", i);
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

        static int task_worker(void* data) {
            while (_globals.data->flags.run) {
                task_manager::do_work();
            }

            return 0;
        }


        /*! NOP task.
         */
        static void _task_nop(void*) {}

        /*! Get new task idx from pool.
         * \return Task idx.
         */
        CE_INLINE uint32_t _new_task_from_pool() {
            for (uint32_t i = 0; i < MAX_TASK; ++i) {
                if (_globals.data->_task_pool[i].id.i != 0) {
                    continue;
                }

                return i;
            }

            /* TODO =( */
            log_globals::log().error("task", "Pool overflow");
            abort();
        }

        /*! Find task in pool
         * \param id Task id.
         * \return Task ptr.
         * \retval NULL Task not found.
         */
        CE_INLINE Task* _find_task_in_pool(TaskID id) {
            for (uint32_t i = 0; i < MAX_TASK; ++i) {
                if (_globals.data->_task_pool[i].id.i != id.i) {
                    continue;
                }

                return &_globals.data->_task_pool[i];
            }

            return NULL;
        }


        /*! Mark job as done.
         * \param id Task id.
         */
        void _mark_task_job_done(TaskID id) {
            thread::spin_lock(_globals.data->_lock);

            for (uint32_t i = 0; i < _globals.data->_open_task_count; ++i) {
                Task* t = &_globals.data->_task_pool[_globals.data->_open_task[i]];

                if (t->id.i != id.i) {
                    continue;
                }

                t->id.i = 0;

                if (t->parent.i != 0) {
                    Task* parent_task = _find_task_in_pool(t->parent);
                    --parent_task->job_count;
                }

                _globals.data->_open_task[i] = _globals.data->_open_task[_globals.data->_open_task_count - 1];
                --_globals.data->_open_task_count;
                break;
            }

            thread::spin_unlock(_globals.data->_lock);
        }

        /*! Is task done?
         * \param id Task id.
         * \retval 0 Not done.
         * \retval 1 Done.
         */
        CE_INLINE uint8_t _is_task_done(TaskID id) {
            uint8_t is_done = 1;

            for (uint32_t i = 0; i < _globals.data->_open_task_count; ++i) {
                if (_globals.data->_task_pool[_globals.data->_open_task[i]].id.i != id.i) {
                    continue;
                }

                is_done = 0;
                break;
            }

            return is_done;
        }

        int8_t task_is_null(TaskID id) {
            return id.i == 0;
        }

        uint32_t task_get_worker_id() {
            uint32_t id = thread::id();

            for (uint8_t i = 0; i < array::size(_globals.data->_workers); ++i) {
                if (thread::get_id(_globals.data->_workers[i]) != id) {
                    continue;
                }

                return i + 1;
            }

            return 0;
        }

        uint8_t task_is_done(TaskID id) {
            // thread::spin_lock(_lock);
            uint8_t is_done = _is_task_done(id);
            // thread::spin_unlock(_lock);
            return is_done;
        }



        Task task_pop_new_work() {
            thread::spin_lock(_globals.data->_lock);

            if (_globals.data->_task_count < 1) {
                thread::spin_unlock(_globals.data->_lock);
                return (Task)TASK_INITIALIZER;
            }

            for (int32_t i = _globals.data->_task_count - 1; i >= 0; --i) {
                Task t = _globals.data->_task_pool[_globals.data->_task_queue[i]];

                if (t.job_count != 1) {
                    continue;
                }

                if (t.depend.i > 0) {
                    if (!_is_task_done(t.depend)) {
                        continue;
                    }
                }

                for (; i < _globals.data->_task_count; ++i) {
                    _globals.data->_task_queue[i] = _globals.data->_task_queue[i + 1];
                }

                --_globals.data->_task_count;
                CE_ASSERT(_globals.data->_task_count != 4294967295);

                thread::spin_unlock(_globals.data->_lock);
                return t;
            }

            thread::spin_unlock(_globals.data->_lock);
            return (Task)TASK_INITIALIZER;
        }

    }

    namespace task_manager {
        TaskID add_begin(const TaskWorkFce_t fce,
                         void* data,
                         const uint32_t priority,
                         const TaskID depend,
                         const TaskID parent) {
            TaskWorkCallback callback = { fce, data };

            thread::spin_lock(_globals.data->_lock);

            const uint32_t id = ++_globals.data->_last_id;

            uint32_t task = _new_task_from_pool();

            /* First? */
            if (_globals.data->_task_count == 0) {
                Task t;
                t.id = { id },
                t.priority = priority,
                t.clb = callback,
                t.job_count = 2,
                t.depend = depend,
                t.parent = parent,

                _globals.data->_task_pool[task] = t;

                _globals.data->_task_queue[0] = task;
            } else {

                /* push item to queue */
                uint32_t i = _globals.data->_task_count;

                while (i > 0 && _globals.data->_task_pool[_globals.data->_task_queue[i - 1]].priority > priority) {
                    _globals.data->_task_queue[i] = _globals.data->_task_queue[i - 1];
                    --i;
                }

                Task t;
                t.id = { id },
                t.priority = priority,
                t.clb = callback,
                t.job_count = 2,
                t.depend = depend,
                t.parent = parent,

                _globals.data->_task_pool[task] = t;

                _globals.data->_task_queue[i] = task;
            }

            if (parent.i != 0) {
                Task* t = _find_task_in_pool(parent);
                ++t->job_count;
            }

            _globals.data->_open_task[_globals.data->_open_task_count] = task;
            ++_globals.data->_open_task_count;
            ++_globals.data->_task_count;

            thread::spin_unlock(_globals.data->_lock);
            return (TaskID) {
                       id
            };

        }

        TaskID add_empty_begin(const uint32_t priority, const TaskID depend, const TaskID parent) {
            return add_begin(_task_nop, 0, priority, depend, parent);
        }

        void add_end(const TaskID* tasks, const uint32_t count) {
            thread::spin_lock(_globals.data->_lock);

            for (uint32_t i = 0; i < MAX_TASK; ++i) {
                for (uint32_t j = 0; j < count; ++j) {
                    if (_globals.data->_task_pool[i].id.i != tasks[j].i) {
                        continue;
                    }

                    --_globals.data->_task_pool[i].job_count;
                    break;
                }
            }

            thread::spin_unlock(_globals.data->_lock);
        }

        void do_work() {
            Task t = task_pop_new_work();

            if (t.id.i == 0) {
                return;
            }

            CE_ASSERT(t.clb.fce != NULL);

            t.clb.fce(t.clb.data);

            _mark_task_job_done(t.id);
        }

        void wait(const TaskID id) {
            while (_globals.data->flags.run && (array::size(_globals.data->_workers) > 0) &&
                   !task_is_done(id)) {
                do_work();
            }
        }

        void spawn_workers() {
            uint32_t core_count = cpu::core_count() * 2;

            static const uint32_t main_threads_count = 1 + 1;
            const uint32_t worker_count = core_count - main_threads_count;

            log_globals::log().info("task", "Core count: %u", core_count);
            log_globals::log().info("task", "Main thread count: %u", main_threads_count);
            log_globals::log().info("task", "Worker count: %u", worker_count);

            if (worker_count > 0) {
                for (uint32_t i = 0; i < worker_count; ++i) {
                    log_globals::log().debug("task", "Creating worker %u.", i);
                    array::push_back(_globals.data->_workers, thread::create_thread(task_worker, "Worker", 0));
                }
            }

            _globals.data->flags.run = 1;
        }

        void stop() {
            _globals.data->flags.run = 0;
        }

    }

    namespace task_manager_globals {
        void init() {
            log_globals::log().info("task_manager_globals", "Init");
            char* p = _globals.buffer;
            _globals.data = new(p) TaskManagerData(memory_globals::default_allocator());

            spawn_workers();
        }

        void shutdown() {
            log_globals::log().info("task_manager_globals", "Shutdown");

            _globals.data->~TaskManagerData();
            _globals = Globals();
        }
    }

}