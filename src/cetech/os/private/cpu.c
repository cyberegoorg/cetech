#include <include/SDL2/SDL_cpuinfo.h>
#include <cetech/os/cpu.h>
#include <cetech/module/module.h>
#include <cetech/api/api_system.h>
#include "celib/macros.h"

int cpu_count() {
    return SDL_GetCPUCount();
}

static struct ct_cpu_a0 cpu_api = {
        .count = cpu_count
};

CETECH_MODULE_DEF(
        cpu,
        {
            CEL_UNUSED(api);
        },
        {
            CEL_UNUSED(reload);
            api->register_api("ct_cpu_a0", &cpu_api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
        }
)
