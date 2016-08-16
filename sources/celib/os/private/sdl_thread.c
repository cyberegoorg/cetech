#include <celib/os/thread.h>

#include "include/SDL2/SDL.h"

thread_t thread_create(thread_fce_t fce,
                       const char *name,
                       void *data) {
    return (thread_t) {
            .t = (void*) SDL_CreateThread(fce, name, data)
    };
}

void thread_kill(thread_t thread) {
    SDL_DetachThread(thread.t);
}

void thread_wait(thread_t thread,
                 int *status) {
    SDL_WaitThread(thread.t, status);
}

u64 thread_get_id(thread_t thread) {
    return SDL_GetThreadID(thread.t);
}

u64 thread_actual_id() {
    return SDL_ThreadID();
}