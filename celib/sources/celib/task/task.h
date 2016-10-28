#ifndef CETECH_TASKMANAGER_H
#define CETECH_TASKMANAGER_H

//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <stdatomic.h>
#include "celib/task/types.h"

#define CE_ASSERT_IS_WORKER(where, worker_id)\
    CE_ASSERT_MSG(where, taskmanager_worker_id() == worker_id, "Current worker: %d", taskmanager_worker_id())

//==============================================================================
// Interface
//==============================================================================

int taskmanager_init(int stage);

void taskmanager_shutdown();

int taskmanager_worker_count();

void taskmanager_add(const char *name,
                     task_work_t work,
                     void *data,
                     enum task_affinity affinity);

int taskmanager_do_work();

void taskmanager_wait_atomic(atomic_int *signal, u32 value);

char taskmanager_worker_id();

#endif //CETECH_TASKMANAGER_H
