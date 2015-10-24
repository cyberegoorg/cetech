#pragma once

#include <cstdint>
#include <string.h>

#include "celib/memory/memory_types.h"

#define NULL_TASK {0}

namespace cetech {
    namespace task_manager {
        typedef void (* TaskWorkFce_t)(void*);
        struct TaskID {
            uint32_t i;
        };

        TaskID add_begin(const TaskWorkFce_t fce,
                         void* data,
                         const uint32_t priority,
                         const TaskID depend = NULL_TASK,
                         const TaskID parent = NULL_TASK);

        TaskID add_empty_begin(const uint32_t priority,
                               const TaskID depend = NULL_TASK,
                               const TaskID parent = NULL_TASK);

        void add_end(const TaskID* tasks, const uint32_t count);
        void wait(const TaskID id);
        void do_work();
    }

    namespace task_manager_globals {
        void init();
        void shutdown();
    }
}