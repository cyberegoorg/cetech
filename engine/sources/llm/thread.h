#ifndef CETECH_THREAD_H
#define CETECH_THREAD_H

#include <celib/types.h>
#include "thread_types.h"

//! Create new thread
//! \param fce Thread fce
//! \param name Thread name
//! \param data Thread data
//! \return new thread
thread_t llm_thread_create(thread_fce_t fce,
                           const char *name,
                           void *data);

//! Kill thread
//! \param thread thread
void llm_thread_kill(thread_t thread);

//! Wait for thread
//! \param thread Thread
//! \param status Thread exit status
void llm_thread_wait(thread_t thread,
                     int *status);

//! Get id for thread
//! \param thread Thread
//! \return ID
u64 llm_thread_get_id(thread_t thread);


//! Get actual thread id
//! \return Thread id
u64 llm_thread_actual_id();

void llm_thread_yield();

void llm_spin_lock(spinlock_t *lock);

void llm_spin_unlock(spinlock_t *lock);


#endif //CETECH_THREAD_H
