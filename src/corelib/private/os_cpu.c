#include <include/SDL2/SDL_cpuinfo.h>
#include <corelib/os.h>
#include <corelib/module.h>
#include <corelib/api_system.h>
#include "corelib/macros.h"

int cpu_count() {
    return SDL_GetCPUCount();
}

static struct ct_cpu_a0 cpu_api = {
        .count = cpu_count
};

struct ct_cpu_a0 *ct_cpu_a0 = &cpu_api;

CETECH_MODULE_DEF(
        cpu,
        {
            CT_UNUSED(api);
        },
        {
            CT_UNUSED(reload);
            api->register_api("ct_cpu_a0", ct_cpu_a0);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
        }
)
