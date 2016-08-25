#include "include/SDL2/SDL_cpuinfo.h"

int llm_cpu_count() {
    return SDL_GetCPUCount();
}