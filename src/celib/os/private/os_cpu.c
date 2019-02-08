#include <include/SDL2/SDL_cpuinfo.h>

#include <celib/module.h>
#include <celib/api.h>
#include "celib/macros.h"
#include <celib/os/cpu.h>

int cpu_count() {
    return SDL_GetCPUCount();
}

struct ce_os_cpu_a0 cpu_api = {
        .count = cpu_count
};

struct ce_os_cpu_a0 *ce_os_cpu_a0 = &cpu_api;
