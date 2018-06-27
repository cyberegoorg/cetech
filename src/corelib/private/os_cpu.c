#include <include/SDL2/SDL_cpuinfo.h>
#include <corelib/os.h>
#include <corelib/module.h>
#include <corelib/api_system.h>
#include "corelib/macros.h"

int cpu_count() {
    return SDL_GetCPUCount();
}

struct ct_cpu_a0 cpu_api = {
        .count = cpu_count
};
