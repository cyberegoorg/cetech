#include <celib/os/thread_types.h>
#include <celib/types.h>
#include "include/SDL2/SDL.h"

thread_t os_thread_create(thread_fce_t fce,
                          const char *name,
                          void *data) {
    return (thread_t) {
            .t = (void *) SDL_CreateThread(fce, name, data)
    };
}

void os_thread_kill(thread_t thread) {
    SDL_DetachThread(thread.t);
}

void os_thread_wait(thread_t thread,
                    int *status) {
    SDL_WaitThread(thread.t, status);
}

u64 os_thread_get_id(thread_t thread) {
    return SDL_GetThreadID(thread.t);
}

u64 os_thread_actual_id() {
    return SDL_ThreadID();
}

void os_thread_spin_lock(spinlock_t *lock) {
    SDL_AtomicLock((SDL_SpinLock *) lock);
}

void os_thread_spin_unlock(spinlock_t *lock) {
    SDL_AtomicUnlock((SDL_SpinLock *) lock);
}
