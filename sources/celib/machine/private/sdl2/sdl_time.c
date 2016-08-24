#include "include/SDL2/SDL_timer.h"
#include "celib/types.h"

u32 machine_get_ticks() {
    return SDL_GetTicks();
}