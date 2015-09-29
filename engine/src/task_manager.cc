#include "task_manager.h"

#include "common/memory/memory.h"
#include "common/container/array.inl.h"

#include "device.h"

#include "os/os.h"

#define TASK_INITIALIZER { { 0 }, { 0 }, { 0 }, 0, 0, { 0, 0 } }

namespace cetech {
    //TODO: REWRITE !!!
    class TaskManagerImplementation : public TaskManager {
        public:
            friend class TaskManager;

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

            os::Spinlock _lock;               //!< Spinlock

            uint32_t _last_id;                //!< Last id
            uint32_t _task_count;             //!< Task count
            uint32_t _open_task_count;        //!< Open task count

            uint32_t _task_queue[MAX_TASK];   //!< Task queue
            uint32_t _open_task[MAX_TASK];    //!< Open task
            Task _task_pool[MAX_TASK];        //!< Task pool

            Array < os::Thread > _workers;


            TaskManagerImplementation(Allocator & allocator) : _last_id(0), _task_count(0), _open_task_count(0), _workers(allocator) {
                memset(_task_queue, 0, sizeof(uint32_t)*MAX_TASK);
                memset(_open_task, 0, sizeof(uint32_t)*MAX_TASK);
                memset(_task_pool, 0, sizeof(Task)*MAX_TASK);
                
                uint32_t core_count = os::cpu::core_count();

                const uint32_t main_threads_count = 2;
                const uint32_t worker_count = core_count - main_threads_count;

                log::info("task", "Core count: %u", core_count);
                log::info("task", "Main thread count: %u", main_threads_count);
                log::info("task", "Worker count: %u", worker_count);

                if (worker_count > 0) {
                    for (uint32_t i = 0; i < worker_count; ++i) {
                        log::debug("task", "Creating worker %u.", i);
                        array::push_back(_workers, os::thread::create_thread(task_worker, "Worker", this));
                    }
                }
            }

            virtual ~TaskManagerImplementation() {
                for (uint32_t i = 0; i < array::size(_workers); ++i) {
                    log::debug("task", "Killing worker%u.", i);
                    os::thread::kill(_workers[i]);
                }
            };

            
            
            virtual TaskID add_begin(const TaskWorkFce_t fce, void* data, const uint32_t priority, const TaskID depend, const TaskID parent) final {
                TaskWorkCallback callback = { fce, data };
                
                os::thread::spin_lock(_lock);

                const uint32_t id = ++this->_last_id;

                uint32_t task = _new_task_from_pool();
                
                /* First? */
                if (this->_task_count == 0) {
                    Task t;
                    t.id = { id },
                    t.priority = priority,
                    t.clb = callback,
                    t.job_count = 2,
                    t.depend = depend,
                    t.parent = parent,

                    this->_task_pool[task] = t;

                    this->_task_queue[0] = task;
                } else {

                    /* push item to queue */
                    uint32_t i = this->_task_count;

                    while (i > 0 && this->_task_pool[this->_task_queue[i - 1]].priority > priority) {
                        this->_task_queue[i] = this->_task_queue[i - 1];
                        --i;
                    }

                    Task t;
                    t.id = { id },
                    t.priority = priority,
                    t.clb = callback,
                    t.job_count = 2,
                    t.depend = depend,
                    t.parent = parent,

                    this->_task_pool[task] = t;

                    this->_task_queue[i] = task;
                }

                if (parent.i != 0) {
                    Task* t = _find_task_in_pool(parent);
                    ++t->job_count;
                }

                this->_open_task[this->_open_task_count] = task;
                ++this->_open_task_count;
                ++this->_task_count;

                 os::thread::spin_unlock(_lock);;
                return (TaskID) {
                           id
                };
            }

            virtual TaskID add_empty_begin(const uint32_t priority, const TaskID depend, const TaskID parent) final {
                return add_begin(_task_nop, 0, priority, depend, parent);
            }

            virtual void add_end(const TaskID* tasks, const uint32_t count) final {
                 os::thread::spin_lock(_lock);

                for (uint32_t i = 0; i < MAX_TASK; ++i) {
                    for (uint32_t j = 0; j < count; ++j) {
                        if (this->_task_pool[i].id.i != tasks[j].i) {
                            continue;
                        }

                        --this->_task_pool[i].job_count;
                        break;
                    }
                }

                 os::thread::spin_unlock(_lock);
            }
            
            virtual void wait(const TaskID id) final {
                while ( /*app_is_run() &&*/ (array::size(_workers) > 0) && !task_is_done(id)) {
                    Task t = task_pop_new_work();
                    
                    if (t.id.i == 0) {
                        continue;
                    }

                    CE_ASSERT(t.clb.fce != NULL);

                    t.clb.fce(t.clb.data);

                    _mark_task_job_done(t.id);
                }
            }
            
            /*! NOP task.
             */
            static void _task_nop(void*) {
            }

            /*! Get new task idx from pool.
             * \return Task idx.
             */
            CE_INLINE uint32_t _new_task_from_pool() {
                for (uint32_t i = 0; i < MAX_TASK; ++i) {
                    if (this->_task_pool[i].id.i != 0) {
                        continue;
                    }

                    return i;
                }

                /* TODO =( */
                log::error("task", "Pool overflow");
                abort();
            }

            /*! Find task in pool
             * \param id Task id.
             * \return Task ptr.
             * \retval NULL Task not found.
             */
            CE_INLINE Task* _find_task_in_pool(TaskID id) {
                for (uint32_t i = 0; i < MAX_TASK; ++i) {
                    if (this->_task_pool[i].id.i != id.i) {
                        continue;
                    }

                    return &this->_task_pool[i];
                }

                return NULL;
            }


            /*! Mark job as done.
             * \param id Task id.
             */
            void _mark_task_job_done(TaskID id) {
                // os::thread::spin_lock(_lock);

                for (uint32_t i = 0; i < this->_open_task_count; ++i) {
                    Task* t = &this->_task_pool[this->_open_task[i]];

                    if (t->id.i != id.i) {
                        continue;
                    }

                    t->id.i = 0;

                    if (t->parent.i != 0) {
                        Task* parent_task = _find_task_in_pool(t->parent);
                        --parent_task->job_count;
                    }

                    this->_open_task[i] = this->_open_task[this->_open_task_count - 1];
                    --this->_open_task_count;
                    break;
                }

                // os::thread::spin_unlock(_lock);;
            }

            /*! Is task done?
             * \param id Task id.
             * \retval 0 Not done.
             * \retval 1 Done.
             */
            CE_INLINE uint8_t _is_task_done(TaskID id) {
                uint8_t is_done = 1;

                for (uint32_t i = 0; i < this->_open_task_count; ++i) {
                    if (this->_task_pool[this->_open_task[i]].id.i != id.i) {
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
                uint32_t id = os::thread::id();

                for (uint8_t i = 0; i < array::size(this->_workers); ++i) {
                    if (os::thread::get_id(this->_workers[i]) != id) {
                        continue;
                    }

                    return i + 1;
                }

                return 0;
            }

            uint8_t task_is_done(TaskID id) {
                // os::thread::spin_lock(_lock);
                uint8_t is_done = _is_task_done(id);
                // os::thread::spin_unlock(_lock);;
                return is_done;
            }



            Task task_pop_new_work() {
                 os::thread::spin_lock(_lock);

                if (this->_task_count < 1) {
                     os::thread::spin_unlock(_lock);;
                    return (Task)TASK_INITIALIZER;
                }

                for (int32_t i = this->_task_count - 1; i >= 0; --i) {
                    Task t = this->_task_pool[this->_task_queue[i]];

                    if (t.job_count != 1) {
                        continue;
                    }

                    if (t.depend.i > 0) {
                        if (!_is_task_done(t.depend)) {
                            continue;
                        }
                    }

                    for (; i < this->_task_count; ++i) {
                        this->_task_queue[i] = this->_task_queue[i + 1];
                    }

                    --this->_task_count;
                    CE_ASSERT(this->_task_count != 4294967295);
                    
                     os::thread::spin_unlock(_lock);;
                    return t;
                }

                 os::thread::spin_unlock(_lock);;
                return (Task)TASK_INITIALIZER;
            }

            static int task_worker(void* data) {
                TaskManagerImplementation* tm = (TaskManagerImplementation*) data;

                while (1) {
                    Task t = tm->task_pop_new_work();
                    
                    if (t.id.i == 0) {
                        continue;
                    }

                    CE_ASSERT(t.clb.fce != NULL);

                    t.clb.fce(t.clb.data);

                    tm->_mark_task_job_done(t.id);
                }

                return 0;
            }
    };

    TaskManager* TaskManager::make(Allocator& alocator) {
        return MAKE_NEW(alocator, TaskManagerImplementation, alocator);
    }

    void TaskManager::destroy(Allocator& alocator, TaskManager* rm) {
        MAKE_DELETE(memory_globals::default_allocator(), TaskManager, rm);
    }
}