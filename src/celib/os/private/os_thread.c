#include "include/SDL2/SDL.h"
#include <celib/platform.h>

#if CE_PLATFORM_LINUX
#include <sched.h>
#endif

#if CE_PLATFORM_OSX

#include <sched.h>

#endif


#include <celib/module.h>
#include <celib/os/thread.h>


//! Create new thread
//! \param fce Thread fce
//! \param name Thread name
//! \param data Thread data
//! \return new thread
ce_thread_t0 thread_create(ce_thread_fce_t0 fce,
                           const char *name,
                           void *data) {
    return (ce_thread_t0) {.o=(uint64_t) SDL_CreateThread(fce, name, data)};
}

//! Kill thread
//! \param thread thread
void thread_kill(ce_thread_t0 thread) {
    SDL_DetachThread((SDL_Thread *) thread.o);
}

//! Wait for thread
//! \param thread Thread
//! \param status Thread exit status
void thread_wait(ce_thread_t0 thread,
                 int *status) {
    SDL_WaitThread((SDL_Thread *) thread.o, status);
}

//! Get id for thread
//! \param thread Thread
//! \return ID
uint64_t thread_get_id(ce_thread_t0 thread) {
    return SDL_GetThreadID((SDL_Thread *) thread.o);
}

//! Get actual thread id
//! \return Thread id
uint64_t thread_actual_id() {
    return SDL_ThreadID();
}

#include <unistd.h>

void thread_yield() {
#if CE_PLATFORM_OSX || CE_PLATFORM_LINUX
//    usleep(0);
    sched_yield();
#else
#error "Implement this"
#endif
}

void thread_spin_lock(ce_spinlock_t0 *lock) {
    SDL_AtomicLock((SDL_SpinLock *) lock);
}

void thread_spin_unlock(ce_spinlock_t0 *lock) {
    SDL_AtomicUnlock((SDL_SpinLock *) lock);
}

struct ce_os_thread_a0 thread_api = {
        .create = thread_create,
        .kill = thread_kill,
        .wait = thread_wait,
        .get_id = thread_get_id,
        .actual_id = thread_actual_id,
        .yield = thread_yield,
        .spin_lock = thread_spin_lock,
        .spin_unlock = thread_spin_unlock
};

struct ce_os_thread_a0 *ce_os_thread_a0 = &thread_api;
