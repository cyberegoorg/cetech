#ifndef CELIB_TIME_H
#define CELIB_TIME_H

#include "types.h"

#if defined(CELIB_USE_SDL)

#include "include/SDL2/SDL.h"

#endif

//==============================================================================
// Time
// ==============================================================================


uint32_t cel_get_ticks();

uint64_t cel_get_perf_counter();

uint64_t cel_get_perf_freq();

#endif //CELIB_TIME_H
