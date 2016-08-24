#ifndef CETECH_TASKMANAGER_H
#define CETECH_TASKMANAGER_H

//==============================================================================
// Includes
//==============================================================================

#include "types.h"

//==============================================================================
// Interface
//==============================================================================

int taskmanager_init();

void taskmanager_shutdown();

task_t taskmanager_add_begin(const char *name,
                             task_work_t work,
                             void *data,
                             char data_len,
                             task_t depend,
                             task_t parent,
                             enum task_priority priority,
                             enum task_affinity affinity);

task_t taskmanager_add_null(const char *name,
                            task_t depend,
                            task_t parent,
                            enum task_priority priority,
                            enum task_affinity affinity);

void taskmanager_add_end(const task_t *tasks, size_t count);

void taskmanager_do_work();

void taskmanager_wait(task_t task);

char taskmanager_worker_id();

int taskmanager_open_task_count();

#endif //CETECH_TASKMANAGER_H
