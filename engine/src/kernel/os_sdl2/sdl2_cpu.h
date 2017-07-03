#include "include/SDL2/SDL.h"

int cpu_count() {
    return SDL_GetCPUCount();
}