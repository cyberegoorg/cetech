#if defined(CELIB_USE_SDL)

#include <celib/types.h>
#include "include/SDL2/SDL.h"

#endif

//==============================================================================
// Time
// ==============================================================================


uint32_t cel_get_ticks() {
#if defined(CELIB_USE_SDL)
    return SDL_GetTicks();
#endif
}

uint64_t cel_get_perf_counter() {
#if defined(CELIB_USE_SDL)
    return SDL_GetPerformanceCounter();
#endif
}

uint64_t cel_get_perf_freq() {
#if defined(CELIB_USE_SDL)
    return SDL_GetPerformanceFrequency();
#endif
}