#include "llm/llm.h"

#include "include/SDL2/SDL.h"

thread_t llm_thread_create(thread_fce_t fce,
                           const char *name,
                           void *data) {
    return (thread_t) {
            .t = (void *) SDL_CreateThread(fce, name, data)
    };
}

void llm_thread_kill(thread_t thread) {
    SDL_DetachThread(thread.t);
}

void llm_thread_wait(thread_t thread,
                     int *status) {
    SDL_WaitThread(thread.t, status);
}

u64 llm_thread_get_id(thread_t thread) {
    return SDL_GetThreadID(thread.t);
}

u64 llm_thread_actual_id() {
    return SDL_ThreadID();
}

void llm_spin_lock(spinlock_t *lock) {
    SDL_AtomicLock((SDL_SpinLock *) lock);
}

void llm_spin_unlock(spinlock_t *lock) {
    SDL_AtomicUnlock((SDL_SpinLock *) lock);
}
