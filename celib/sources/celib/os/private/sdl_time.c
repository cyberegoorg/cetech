#include "include/SDL2/SDL_timer.h"
#include "../../types.h"

u32 os_get_ticks() {
    return SDL_GetTicks();
}