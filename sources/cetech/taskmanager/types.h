#ifndef CETECH_TASK_MANAGER_TYPES_H
#define CETECH_TASK_MANAGER_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/types.h"

//==============================================================================
// Types
//==============================================================================


typedef struct {
    u32 id;
} task_t;

static const task_t task_null = (task_t){.id = 0};

typedef void (*task_work_t)(void* data);

enum task_affinity{
    TASK_AFFINITY_NONE    = 0,
    TASK_AFFINITY_MAIN    = 1,
    TASK_AFFINITY_WORKER1 = 2,
    TASK_AFFINITY_WORKER2 = 3,
    TASK_AFFINITY_WORKER3 = 4,
    TASK_AFFINITY_WORKER4 = 5,
    TASK_AFFINITY_WORKER5 = 6,
    TASK_AFFINITY_WORKER6 = 7,
    TASK_AFFINITY_WORKER7 = 8,
    MAX_WORKERS = TASK_AFFINITY_WORKER7,
};

enum task_priority{
    TASK_PRIORITY_HIGH   = 0,
    TASK_PRIORITY_NORMAL = 1,
    TASK_PRIORITY_LOW    = 2,
    TASK_PRIORITY_COUNT  = 3,

};

#endif //CETECH_TASK_MANAGER_TYPES_H
