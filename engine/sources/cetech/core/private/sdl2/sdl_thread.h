#include "../../thread.h"

#include "include/SDL2/SDL.h"

#if defined(CETECH_LINUX)

#include <unistd.h>
#include <sched.h>

#endif

//! Create new thread
//! \param fce Thread fce
//! \param name Thread name
//! \param data Thread data
//! \return new thread
thread_t thread_create(thread_fce_t fce,
                           const char *name,
                           void *data) {
    return (thread_t) {
            .t = (void *) SDL_CreateThread(fce, name, data)
    };
}

//! Kill thread
//! \param thread thread
void thread_kill(thread_t thread) {
    SDL_DetachThread(thread.t);
}

//! Wait for thread
//! \param thread Thread
//! \param status Thread exit status
void thread_wait(thread_t thread,
                     int *status) {
    SDL_WaitThread(thread.t, status);
}

//! Get id for thread
//! \param thread Thread
//! \return ID
uint64_t thread_get_id(thread_t thread) {
    return SDL_GetThreadID(thread.t);
}

//! Get actual thread id
//! \return Thread id
uint64_t thread_actual_id() {
    return SDL_ThreadID();
}

void thread_yield() {
#if defined(CETECH_DARWIN)
    sched_yield();
#elif defined(CETECH_LINUX)
    sched_yield();
#endif
}

void thread_spin_lock(spinlock_t *lock) {
    SDL_AtomicLock((SDL_SpinLock *) lock);
}

void thread_spin_unlock(spinlock_t *lock) {
    SDL_AtomicUnlock((SDL_SpinLock *) lock);
}
