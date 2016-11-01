#ifndef CELIB_TASK_TYPES_H
#define CELIB_TASK_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/types.h"


typedef int (*thread_fce_t)(void *data);

typedef struct {
    void *t;
} thread_t;

typedef struct {
    int lock;
} spinlock_t;


#endif //CELIB_TASK_TYPES_H
