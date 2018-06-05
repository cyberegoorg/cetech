#include <include/SDL2/SDL_cpuinfo.h>
#include <cetech/kernel/os/cpu.h>
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/api/api_system.h>
#include "cetech/kernel/macros.h"

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
