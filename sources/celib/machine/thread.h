#ifndef CETECH_THREAD_H
#define CETECH_THREAD_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/types.h"
#include "types.h"


//==============================================================================
// Includes
//==============================================================================

//! Create new thread
//! \param fce Thread fce
//! \param name Thread name
//! \param data Thread data
//! \return new thread
thread_t thread_create(thread_fce_t fce,
                       const char *name,
                       void *data);

//! Kill thread
//! \param thread thread
void thread_kill(thread_t thread);

//! Wait for thread
//! \param thread Thread
//! \param status Thread exit status
void thread_wait(thread_t thread,
                 int *status);

//! Get id for thread
//! \param thread Thread
//! \return ID
u64 thread_get_id(thread_t thread);


//! Get actual thread id
//! \return Thread id
u64 thread_actual_id();


void spin_lock(spinlock_t *lock);

void spin_unlock(spinlock_t *lock);

#endif //CETECH_THREAD_H
