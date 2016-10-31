#ifndef CELIB_THREAD_H
#define CELIB_THREAD_H

#include "../types.h"
#include "types.h"


#if defined(CELIB_USE_SDL)

#include "include/SDL2/SDL.h"

#endif

#if defined(CELIB_LINUX)

#include <unistd.h>
#include <sched.h>

#endif

//! Create new thread
//! \param fce Thread fce
//! \param name Thread name
//! \param data Thread data
//! \return new thread
static thread_t cel_thread_create(thread_fce_t fce,
                                 const char *name,
                                 void *data) {
#if defined(CELIB_USE_SDL)
    return (thread_t) {
            .t = (void *) SDL_CreateThread(fce, name, data)
    };
#endif
}

//! Kill thread
//! \param thread thread
static void cel_thread_kill(thread_t thread) {
#if defined(CELIB_USE_SDL)
    SDL_DetachThread(thread.t);
#endif
}

//! Wait for thread
//! \param thread Thread
//! \param status Thread exit status
static void cel_thread_wait(thread_t thread,
                           int *status) {
#if defined(CELIB_USE_SDL)
    SDL_WaitThread(thread.t, status);
#endif
}

//! Get id for thread
//! \param thread Thread
//! \return ID
static u64 cel_thread_get_id(thread_t thread) {
#if defined(CELIB_USE_SDL)
    return SDL_GetThreadID(thread.t);
#endif
}

//! Get actual thread id
//! \return Thread id
static u64 cel_thread_actual_id() {
#if defined(CELIB_USE_SDL)
    return SDL_ThreadID();
#endif
}

static void cel_thread_yield() {
#if defined(CELIB_USE_SDL)
    usleep(0);

    return;
#endif

#if defined(CELIB_DARWIN)
    sched_yield();
#elif defined(CELIB_LINUX)
    sched_yield();
#endif
}

static void cel_thread_spin_lock(spinlock_t *lock) {
#if defined(CELIB_USE_SDL)
    SDL_AtomicLock((SDL_SpinLock *) lock);
#endif
}

static void cel_thread_spin_unlock(spinlock_t *lock) {
#if defined(CELIB_USE_SDL)
    SDL_AtomicUnlock((SDL_SpinLock *) lock);
#endif
}


#endif //CELIB_THREAD_H
