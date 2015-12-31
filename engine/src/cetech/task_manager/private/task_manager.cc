#include "cetech/task_manager/task_manager.h"

#include <unistd.h>

#include "celib/types.h"
#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"
#include "celib/container/queue.inl.h"
#include "celib/errors/errors.h"

#include "cetech/thread/thread.h"
#include "cetech/application/application.h"
#include "cetech/log/log.h"

#include "cetech/task_manager/private/queuempmc.h"
#include "cetech/develop/develop_manager.h"

#if defined(CETECH_LINUX)
    #include <pthread.h>
#elif defined(CETECH_DARWIN)
    #include <sched.h>
#endif

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
            TaskWorkCallback clb;               //!< Callback
            std::atomic < uint32_t > job_count; //!< Task child active job

            char name[256];
            Task* depend;              //!< Task depend
            Task* parent;              //!< Task parent

            WorkerAffinity::Enum worker_affinity; //!< Worker affinity
            Priority::Enum priority;              //!< Task priority
            bool used;
        };

        enum {
            MAX_TASK = 4096
        };


        typedef QueueMPMC < Task*, MAX_TASK > TaskQueue;

        static thread_local uint32_t _worker_id = 0; // Worker id (thanks thread_local)
        struct TaskManagerData {
            TaskQueue _global_queue[Priority::Count];
            Array < Thread > _workers;

            uint32_t* _open_task;  //!< Open task
            Task* _task_pool;      //!< Task pool
            TaskQueue* _workers_queue;

            Allocator& _allocator;
            std::atomic < uint32_t > _open_task_count;               //!< Open task count

            struct {
                char run : 1;
            } flags;

            TaskManagerData(Allocator & allocator) : _workers(allocator),
                                                     _workers_queue(nullptr),
                                                     _allocator(allocator),
                                                     _open_task_count(0),
                                                     flags {
                0
            } {

                _open_task = (uint32_t*) allocator.allocate(sizeof(uint32_t) * MAX_TASK);
                _task_pool = memory::alloc_array < Task > (allocator, MAX_TASK);

                memset(_open_task, 0, sizeof(uint32_t) * MAX_TASK);
                //memset(_task_pool, 0, sizeof(Task) * MAX_TASK);
            }

            ~TaskManagerData() {
                stop();

                for (uint32_t i = 0; i < array::size(_workers); ++i) {
                    log::debug("task", "Killing worker %u.", i + 1);
                    thread::wait(_workers[i], 0);
                }

                _allocator.deallocate(_open_task);
                _allocator.deallocate(_task_pool);
                _allocator.deallocate(_workers_queue);
            };
        };

        struct Globals {
            static const int MEMORY = sizeof(TaskManagerData);
            char buffer[MEMORY];

            TaskManagerData* data;

            Globals() : buffer {
                0
            }, data(0) {}
        } _globals;

        uint32_t _core_count() {
#if defined(CETECH_SDL2)
            return SDL_GetCPUCount();
#endif
        }

        static int task_worker(void* data) {
            error_globals::init();

            // Wait for run signal 0 -> 1
            while (_globals.data->flags.run == 0) {}

            _worker_id = (uint64_t)data; // TODO: (uint64_t)?? !!!

            // TODO: multiplatform core afinity... linux ok but mac? no no no
            // #if defined(CETECH_LINUX)
            //             cpu_set_t cpuset;
            //             CPU_ZERO(&cpuset);
            //             CPU_SET(_worker_id, &cpuset);
            //
            //             pthread_t current_thread = pthread_self();
            //             CE_ASSERT(!pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset));
            // #endif

            log::info("task_worker", "Worker%d init", _worker_id);

            while (_globals.data->flags.run) {
                task_manager::do_work();
            }

            log::info("task_worker", "Worker%d shutdown", _worker_id);
            error_globals::shutdown();
            return 0;
        }

        static void _task_nop(void*) {}

        uint32_t _new_task() {
            CE_ASSERT("task_manager", _globals.data != nullptr);
            TaskManagerData& tm = *_globals.data;

            for (uint32_t i = 1; i < MAX_TASK; ++i) {
                if (tm._task_pool[i].used != 0) {
                    continue;
                }

                tm._task_pool[i].used = 1;
                return i;
            }

            CE_ASSERT_MSG("task_manager", false, "Pool overflow");
            abort();
        }

        Task* get_task(TaskID id) {
            return &_globals.data->_task_pool[id.i];
        }

        void push_task(Task* task) {
            CE_ASSERT("task_manager", task->used);
            CE_ASSERT("task_manager", _globals.data != nullptr);

            TaskManagerData& tm = *_globals.data;

            Priority::Enum priority = task->priority;
            WorkerAffinity::Enum aff = task->worker_affinity;

            switch (aff) {
            case WorkerAffinity::NONE:
                queuempmc::push(tm._global_queue[priority], task);
                break;

            default:
                queuempmc::push(tm._workers_queue[((aff - 1) * Priority::Count) + priority], task);
                break;
            }
        }

        bool _task_is_done(Task* task) {
            CE_ASSERT("task_manager", _globals.data != nullptr);
            TaskManagerData& tm = *_globals.data;

            bool ret;
            uint32_t count = 0;

            ret = true;
            count = tm._open_task_count;
            for (uint32_t i = 0; i < count; ++i) {
                if (&tm._task_pool[tm._open_task[i]] != task) {
                    continue;
                }

                ret = false;
                break;
            }

            return ret;
        }

        bool task_is_done(TaskID id) {
            return _task_is_done(get_task(id));
        }

        void mark_task_job_done(Task* task) {
            CE_ASSERT("task_manager", _globals.data != nullptr);
            TaskManagerData& tm = *_globals.data;

            if (task->parent) {
                --task->parent->job_count;
            }

            for (uint32_t i = 0; i < tm._open_task_count; ++i) {
                Task* t = &tm._task_pool[tm._open_task[i]];

                if (t == task) {
                    //--task->job_count;

                    size_t idx = --tm._open_task_count;
                    tm._open_task[i] = tm._open_task[idx];

                    //log::info("task", "%d task %s done.", application::get_frame_id(), task->name);

                    task->used = 0;
                    break;
                }
            }
        }

        //         int8_t task_is_null(TaskID id) {
        //             return id.i == 0;
        //         }

        CE_INLINE bool can_work_on(Task* task) {
            return (task->job_count == 1) && (task->depend ? _task_is_done(task->depend) : 1);
        }

        CE_INLINE Task* try_pop(QueueMPMC < Task*,
                                MAX_TASK >& q) {
            Task* poped_task;

            poped_task = queuempmc::pop(q);
            if (poped_task != 0) {
                CE_ASSERT("task_manager", poped_task->used);

                if (can_work_on(poped_task)) {
                    return poped_task;
                } else {
                    push_task(poped_task);
                    //continue;
                }
            }

            return 0;
        }

        Task* task_pop_new_work() {
            CE_ASSERT("task_manager", _globals.data != nullptr);
            TaskManagerData& tm = *_globals.data;

            Task* poped_task;

            for (int i = 0; i < Priority::Count; ++i) {
                TaskQueue& q = tm._workers_queue[(_worker_id * Priority::Count) + i];

                poped_task = try_pop(q);
                if (poped_task != 0) {
                    return poped_task;
                }
            }

            for (int i = 0; i < Priority::Count; ++i) {
                TaskQueue& q = tm._global_queue[i];

                poped_task = try_pop(q);
                if (poped_task != 0) {
                    return poped_task;
                }
            }

            return 0;
        }
    }

    namespace task_manager {
        uint32_t get_worker_id() {
            return _worker_id;
        }

        uint32_t open_task_count() {
            TaskManagerData& tm = *_globals.data;
            return tm._open_task_count;
        }

        TaskID add_begin(const char* name,
                         const TaskWorkFce_t fce,
                         void* data,
                         const Priority::Enum priority,
                         const TaskID depend,
                         const TaskID parent,
                         const WorkerAffinity::Enum worker_affinity) {

            CE_ASSERT("task_manager", _globals.data != nullptr);
            TaskManagerData& tm = *_globals.data;

            TaskWorkCallback callback = { fce, data };

            uint32_t task = _new_task();

            Task& t = tm._task_pool[task];
            strncpy(t.name, name, 255);

            t.priority = priority,
            t.clb = callback,
            t.job_count = 2,
            t.depend = depend.i != 0 ? get_task(depend) : 0;
            t.parent = parent.i != 0 ? get_task(parent) : 0;
            t.worker_affinity = worker_affinity;

            if (parent.i != 0) {
                Task* paren_task = get_task(parent);
                ++paren_task->job_count;
            }

            const size_t idx = tm._open_task_count++;
            tm._open_task[idx] = task;


            return (TaskID) {
                       task
            };

        }

        TaskID add_empty_begin(const char* name,
                               const Priority::Enum priority,
                               const TaskID depend,
                               const TaskID parent,
                               const WorkerAffinity::Enum worker_affinity) {
            return add_begin(name, _task_nop, 0, priority, depend, parent, worker_affinity);
        }

        void add_end(const TaskID* tasks,
                     const uint32_t count) {
            for (uint32_t i = 0; i < count; ++i) {
                Task* t = get_task(tasks[i]);
                --t->job_count;
            }

            for (uint32_t i = 0; i < count; ++i) {
                Task* t = get_task(tasks[i]);

                push_task(t);
            }
        }

        void do_work() {
            Task* t = task_pop_new_work();

            if (t == 0) {
                //usleep(0);

                #if defined(CETECH_DARWIN)
                sched_yield();
                #elif defined(CETECH_LINUX)
                pthread_yield();
                #endif
                return;
            }

            CE_ASSERT("task_manager", t->clb.fce != NULL);

            auto time = develop_manager::enter_scope(t->name);
            t->clb.fce(t->clb.data);
            develop_manager::leave_scope(t->name, time);

            mark_task_job_done(t);
        }

        void wait(const TaskID id) {
            //auto time = develop_manager::push_enter_scope("TaskManager::wait()");
            while (!task_is_done(id)) {
                do_work();
            }

            //develop_manager::push_leave_scope("TaskManager::wait()", time);
        }

        void spawn_workers() {
            CE_ASSERT("task_manager", _globals.data != nullptr);
            TaskManagerData& tm = *_globals.data;

            uint32_t core_count = _core_count();

            static const uint32_t main_threads_count = 1 + 1;
            const uint32_t worker_count = core_count - main_threads_count;

            log::info("task", "Core count: %u", core_count);
            log::info("task", "Main thread count: %u", main_threads_count);
            log::info("task", "Worker count: %u", worker_count);

            tm._workers_queue = memory::alloc_array < TaskQueue > (tm._allocator, (worker_count + 1) * Priority::Count);

            if (worker_count > 0) {
                for (uint32_t i = 0; i < worker_count; ++i) {
                    log::debug("task", "Creating worker %u.", i + 1);
                    array::push_back(tm._workers,
                                     thread::create_thread(task_worker,
                                                           "Worker",
                                                           (void*)(intptr_t)(i + 1))); // TODO: wtf =D
                }
            }

            tm.flags.run = 1;
        }

        void stop() {
            CE_ASSERT("task_manager", _globals.data != nullptr);
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
