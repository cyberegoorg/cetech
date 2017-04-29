#include "include/SDL2/SDL.h"
#include "cetech/cpu.h"

int cel_cpu_count() {
    return SDL_GetCPUCount();
}