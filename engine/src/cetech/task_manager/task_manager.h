#pragma once

#include <cstdint>
#include <string.h>

#include "celib/memory/memory_types.h"

#define NULL_TASK {0}

namespace cetech {
    namespace task_manager {
        struct WorkerAffinity {
            enum Enum {
                NONE = 0,
                MAIN_THEAD,
                WORKER1,
                WORKER2,
                WORKER3,
                WORKER4,
                WORKER5,
                WORKER6,
                WORKER7,
            };
        };

        typedef void (* TaskWorkFce_t)(void*);
        struct TaskID {
            uint32_t i;
        };

        TaskID add_begin(const TaskWorkFce_t fce,
                         void* data,
                         const uint32_t priority,
                         const TaskID depend = NULL_TASK,
                         const TaskID parent = NULL_TASK,
                         const WorkerAffinity::Enum worker_affinity = WorkerAffinity::NONE);

        TaskID add_empty_begin(const uint32_t priority,
                               const TaskID depend = NULL_TASK,
                               const TaskID parent = NULL_TASK,
                               const WorkerAffinity::Enum worker_affinity = WorkerAffinity::NONE);

        void add_end(const TaskID* tasks, const uint32_t count);

        void wait(const TaskID id);

        void do_work();
        uint32_t get_worker_id();

        void stop();
        
        uint32_t open_task_count();
        //printf("count %d\n", tm._open_task_count);
    }

    namespace task_manager_globals {
        void init();
        void shutdown();
    }
}