#include "include/SDL2/SDL_cpuinfo.h"

int os_cpu_count() {
    return SDL_GetCPUCount();
}