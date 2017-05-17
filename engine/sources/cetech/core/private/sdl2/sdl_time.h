#include "include/SDL2/SDL.h"
#include <cetech/core/types.h>

//==============================================================================
// Time
// ==============================================================================

uint32_t get_ticks() {
    return SDL_GetTicks();
}

uint64_t get_perf_counter() {
    return SDL_GetPerformanceCounter();
}

uint64_t get_perf_freq() {
    return SDL_GetPerformanceFrequency();
}