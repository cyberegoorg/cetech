#include <corelib/os.h>
#include <corelib/module.h>
#include <corelib/api_system.h>
#include "include/SDL2/SDL.h"
#include "corelib/macros.h"

//==============================================================================
// Time
// ==============================================================================

uint32_t get_ticks() {
    return SDL_GetTicks();
}

uint64_t get_perf_counter() {
    return SDL_GetPerformanceCounter();
}

uint64_t get_perf_freq() {
    return SDL_GetPerformanceFrequency();
}

static struct ct_time_a0 time_api = {
        .ticks =get_ticks,
        .perf_counter =get_perf_counter,
        .perf_freq =get_perf_freq
};

struct ct_time_a0 *ct_time_a0 = &time_api;

CETECH_MODULE_DEF(
        time,
        {
            CT_UNUSED(api);
        },
        {
            CT_UNUSED(reload);
            api->register_api("ct_time_a0", ct_time_a0);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
        }
)
