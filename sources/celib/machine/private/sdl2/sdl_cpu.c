#include "include/SDL2/SDL_cpuinfo.h"

int machine_cpu_count() {
    return SDL_GetCPUCount();
}