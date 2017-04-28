#ifndef CELIB_THREAD_H
#define CELIB_THREAD_H

#include "types.h"

typedef int (*thread_fce_t)(void *data);

typedef struct {
    void *t;
} thread_t;

typedef struct {
    int lock;
} spinlock_t;


//! Create new thread
//! \param fce Thread fce
//! \param name Thread name
//! \param data Thread data
//! \return new thread
thread_t cel_thread_create(thread_fce_t fce,
                           const char *name,
                           void *data);

//! Kill thread
//! \param thread thread
void cel_thread_kill(thread_t thread);

//! Wait for thread
//! \param thread Thread
//! \param status Thread exit status
void cel_thread_wait(thread_t thread,
                     int *status);

//! Get id for thread
//! \param thread Thread
//! \return ID
uint64_t cel_thread_get_id(thread_t thread);

//! Get actual thread id
//! \return Thread id
uint64_t cel_thread_actual_id();

void cel_thread_yield();

void cel_thread_spin_lock(spinlock_t *lock);

void cel_thread_spin_unlock(spinlock_t *lock);


#endif //CELIB_THREAD_H
