#include "cetech/task_manager/task_manager.h"

#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"
#include "celib/container/queue.inl.h"

#include "cetech/thread/thread.h"
#include "cetech/application/application.h"
#include "cetech/log_system/log_system.h"

#include <unistd.h>

#include "cetech/task_manager/private/taskqueue.h"

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
            TaskWorkCallback clb;      //!< Callback
            Task* depend;              //!< Task depend
            Task* parent;              //!< Task parent

            WorkerAffinity::Enum worker_affinity; //!< Worker affinity
            std::atomic < uint32_t > job_count;   //!< Task child active job
            uint32_t priority;                    //!< Task priority
            bool used;
        };

        enum {
            MAX_TASK = 4096
        };


        static thread_local uint32_t _worker_id = 0; // Worker id (thanks thread_local)
        struct TaskManagerData {
            Array < Thread > _workers;
            uint32_t _open_task[MAX_TASK];  //!< Open task
            Task _task_pool[MAX_TASK];      //!< Task pool
            TaskQueue < Task*, MAX_TASK > _workers_queue[10]; // TODO: dynamic
            TaskQueue < Task*, MAX_TASK > _global_queue;
            Allocator& _allocator;
            std::atomic < uint32_t > _open_task_count;               //!< Open task count

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

#if defined(CETECH_LINUX)
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(_worker_id, &cpuset);

            pthread_t current_thread = pthread_self();
            CE_ASSERT(!pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset));
#endif

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
                if (tm._task_pool[i].used != 0) {
                    continue;
                }

                tm._task_pool[i].used = 1;
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
            CE_ASSERT(task->used);
            CE_CHECK_PTR(_globals.data);

            TaskManagerData& tm = *_globals.data;

            WorkerAffinity::Enum aff = task->worker_affinity;

            switch (aff) {
            case WorkerAffinity::NONE:
                taskqueue::push(tm._global_queue, task);
                break;

            default:
                taskqueue::push(tm._workers_queue[aff - 1], task);
                break;
            }
        }

        bool _task_is_done(Task* task) {
            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;

            const uint32_t count = tm._open_task_count;
            bool ret = true;

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

        /*! Mark job as done.
         * \param id Task id.
         */
        void mark_task_job_done(Task* task) {
            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;

            if (task->parent) {
                --task->parent->job_count;
            }

            for (uint32_t i = 0; i < tm._open_task_count; ++i) {
                Task* t = &tm._task_pool[tm._open_task[i]];

                if (t == task) {
                    tm._open_task[i] = tm._open_task[tm._open_task_count - 1];
                    --tm._open_task_count;
                    task->used = 0;
                    break;
                }
            }
        }

        int8_t task_is_null(TaskID id) {
            return id.i == 0;
        }



        bool can_work_on(Task* task) {
            if (task->job_count != 1) {
                return false;
            }

            if ((task->depend) && !_task_is_done(task->depend)) {
                return false;
            }

            return true;
        }

        Task* task_pop_new_work() {
            CE_CHECK_PTR(_globals.data);
            TaskManagerData& tm = *_globals.data;


            Task* poped_task;

            TaskQueue < Task*, MAX_TASK >& q = tm._workers_queue[_worker_id];
            poped_task = taskqueue::pop(q);
            if (poped_task != 0) {
                CE_ASSERT(poped_task->used);

                if (can_work_on(poped_task)) {
                    return poped_task;
                } else {
                    push_task(poped_task);
                }
            }

            poped_task = taskqueue::pop(tm._global_queue);

            if (poped_task != 0) {
                CE_ASSERT(poped_task->used);

                if (can_work_on(poped_task)) {
                    return poped_task;
                } else {
                    push_task(poped_task);
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

            Task& t = tm._task_pool[task];
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

                push_task(t);
            }

            for (uint32_t i = 0; i < count; ++i) {
                Task* t = get_task(tasks[i]);
                --t->job_count;
            }
        }

        void do_work() {
            Task* t = task_pop_new_work();

            if (t == 0) {
                usleep(0);
                //pthread_yield();
                return;
            }

            CE_ASSERT(t->clb.fce != NULL);

            t->clb.fce(t->clb.data);

            mark_task_job_done(t);
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
