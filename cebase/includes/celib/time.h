#ifndef CELIB_TIME_H
#define CELIB_TIME_H

#include "types.h"

#if defined(CELIB_USE_SDL)

#include "include/SDL2/SDL.h"

#endif

//==============================================================================
// Time
// ==============================================================================


u32 cel_get_ticks();

u64 cel_get_perf_counter();

u64 cel_get_perf_freq();

#endif //CELIB_TIME_H
