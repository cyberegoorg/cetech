#include "include/SDL2/SDL.h"
#include "cetech/cpu.h"

int cpu_count() {
    return SDL_GetCPUCount();
}