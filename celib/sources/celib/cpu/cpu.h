#ifndef CELIB_CPU_H
#define CELIB_CPU_H

#if defined(CELIB_USE_SDL)

#include "include/SDL2/SDL.h"

#endif

//==============================================================================
// CPU
//==============================================================================

int cel_cpu_count() {
#if defined(CELIB_USE_SDL)
    return SDL_GetCPUCount();
#endif
}

#endif //CELIB_CPU_H
