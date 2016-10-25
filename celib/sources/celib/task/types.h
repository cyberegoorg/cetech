#ifndef CETECH_TASK_TYPES_H
#define CETECH_TASK_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/types.h"

//==============================================================================
// Task
//==============================================================================

typedef struct {
    u32 id;
} task_t;

static const task_t task_null = (task_t) {.id = 0};

typedef void (*task_work_t)(void *data);

enum workers {
    TASK_WORKER_MAIN = 0,
    TASK_WORKER1 = 1,
    TASK_WORKER2 = 2,
    TASK_WORKER3 = 3,
    TASK_WORKER4 = 4,
    TASK_WORKER5 = 5,
    TASK_WORKER6 = 6,
    TASK_WORKER7 = 7,
    TASK_MAX_WORKERS,
};

enum task_affinity {
    TASK_AFFINITY_NONE = 0,
    TASK_AFFINITY_MAIN = 1,
    TASK_AFFINITY_WORKER1 = 2,
    TASK_AFFINITY_WORKER2 = 3,
    TASK_AFFINITY_WORKER3 = 4,
    TASK_AFFINITY_WORKER4 = 5,
    TASK_AFFINITY_WORKER5 = 6,
    TASK_AFFINITY_WORKER6 = 7,
    TASK_AFFINITY_WORKER7 = 8,
    TASK_AFFINITY_MAX
};

CE_STATIC_ASSERT(TASK_AFFINITY_MAX == TASK_MAX_WORKERS + 1);


#endif //CETECH_TASK_TYPES_H
