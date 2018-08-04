#include <include/SDL2/SDL_cpuinfo.h>
#include <celib/os.h>
#include <celib/module.h>
#include <celib/api_system.h>
#include "celib/macros.h"

int cpu_count() {
    return SDL_GetCPUCount();
}

struct ce_os_cpu_a0 cpu_api = {
        .count = cpu_count
};
