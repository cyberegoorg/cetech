#ifndef CELIB_TIME_H
#define CELIB_TIME_H

#include "../types.h"

#if defined(CELIB_USE_SDL)

#include "include/SDL2/SDL.h"

#endif

//==============================================================================
// Time
// ==============================================================================


static u32 celib_get_ticks() {
#if defined(CELIB_USE_SDL)
    return SDL_GetTicks();
#endif
}

static u64 celib_get_perf_counter() {
#if defined(CELIB_USE_SDL)
    return SDL_GetPerformanceCounter();
#endif
}

static u64 celib_get_perf_freq() {
#if defined(CELIB_USE_SDL)
    return SDL_GetPerformanceFrequency();
#endif
}

#endif //CELIB_TIME_H
