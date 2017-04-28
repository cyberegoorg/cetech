#if defined(CELIB_USE_SDL)

#include "include/SDL2/SDL.h"

#endif

int cel_cpu_count() {
#if defined(CELIB_USE_SDL)
    return SDL_GetCPUCount();
#endif
}