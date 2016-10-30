#include "include/SDL2/SDL_timer.h"
#include "../../types.h"

u32 os_get_ticks() {
    return SDL_GetTicks();
}

u64 os_get_perf_counter() {
    return SDL_GetPerformanceCounter();
}

u64 os_get_perf_freq() {
    return SDL_GetPerformanceFrequency();
}