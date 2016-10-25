#ifndef CETECH_TASKMANAGER_H
#define CETECH_TASKMANAGER_H

//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include "celib/task/types.h"

#define CE_ASSERT_IS_WORKER(where, worker_id)\
    CE_ASSERT_MSG(where, taskmanager_worker_id() == worker_id, "Current worker: %d", taskmanager_worker_id())

//==============================================================================
// Interface
//==============================================================================

int taskmanager_init(int stage);

void taskmanager_shutdown();

int taskmanager_worker_count();

task_t taskmanager_add_begin(const char *name,
                             task_work_t work,
                             void *data,
                             char data_len,
                             task_t depend,
                             task_t parent,
                             enum task_affinity affinity);

task_t taskmanager_add_null(const char *name,
                            task_t depend,
                            task_t parent,
                            enum task_affinity affinity);

void taskmanager_add_end(const task_t *tasks,
                         size_t count);

void taskmanager_do_work();

void taskmanager_wait(task_t task);

char taskmanager_worker_id();

#endif //CETECH_TASKMANAGER_H
