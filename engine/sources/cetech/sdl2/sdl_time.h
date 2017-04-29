#include <cetech/types.h>
#include "include/SDL2/SDL.h"

//==============================================================================
// Time
// ==============================================================================

uint32_t cel_get_ticks() {
    return SDL_GetTicks();
}

uint64_t cel_get_perf_counter() {
    return SDL_GetPerformanceCounter();
}

uint64_t cel_get_perf_freq() {
    return SDL_GetPerformanceFrequency();
}