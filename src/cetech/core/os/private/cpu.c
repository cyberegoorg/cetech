#include <include/SDL2/SDL_cpuinfo.h>
#include <cetech/core/os/cpu.h>
#include <cetech/core/module/module.h>
#include <cetech/core/api/api_system.h>
#include "cetech/core/macros.h"

int cpu_count() {
    return SDL_GetCPUCount();
}

static struct ct_cpu_a0 cpu_api = {
        .count = cpu_count
};

CETECH_MODULE_DEF(
        cpu,
        {
            CT_UNUSED(api);
        },
        {
            CT_UNUSED(reload);
            api->register_api("ct_cpu_a0", &cpu_api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
        }
)
