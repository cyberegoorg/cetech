#include "include/SDL2/SDL.h"
#include <corelib/platform.h>

#if CT_PLATFORM_LINUX
#include <sched.h>
#endif

#if CT_PLATFORM_OSX

#include <sched.h>
#include <unistd.h>

#endif

#include <corelib/os.h>
#include <corelib/module.h>
#include <corelib/api_system.h>

#include "corelib/macros.h"

//! Create new thread
//! \param fce Thread fce
//! \param name Thread name
//! \param data Thread data
//! \return new thread
ct_thread_t *thread_create(ct_thread_fce_t fce,
                           const char *name,
                           void *data) {
    return (ct_thread_t *) SDL_CreateThread(fce, name, data);
}

//! Kill thread
//! \param thread thread
void thread_kill(ct_thread_t *thread) {
    SDL_DetachThread((SDL_Thread *) thread);
}

//! Wait for thread
//! \param thread Thread
//! \param status Thread exit status
void thread_wait(ct_thread_t *thread,
                 int *status) {
    SDL_WaitThread((SDL_Thread *) thread, status);
}

//! Get id for thread
//! \param thread Thread
//! \return ID
uint64_t thread_get_id(ct_thread_t *thread) {
    return SDL_GetThreadID((SDL_Thread *) thread);
}

//! Get actual thread id
//! \return Thread id
uint64_t thread_actual_id() {
    return SDL_ThreadID();
}

void thread_yield() {
#if CT_PLATFORM_OSX
    sched_yield();
#elif CT_PLATFORM_LINUX
    sched_yield();
#endif
}

void thread_spin_lock(struct ct_spinlock *lock) {
    SDL_AtomicLock((SDL_SpinLock *) lock);
}

void thread_spin_unlock(struct ct_spinlock *lock) {
    SDL_AtomicUnlock((SDL_SpinLock *) lock);
}

struct ct_thread_a0 thread_api = {
        .create = thread_create,
        .kill = thread_kill,
        .wait = thread_wait,
        .get_id = thread_get_id,
        .actual_id = thread_actual_id,
        .yield = thread_yield,
        .spin_lock = thread_spin_lock,
        .spin_unlock = thread_spin_unlock
};

struct ct_thread_a0 *ct_thread_a0 = &thread_api;
