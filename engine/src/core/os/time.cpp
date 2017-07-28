#include "include/SDL2/SDL.h"
#include <cetech/core/os/time.h>
#include <celib/macros.h>
#include <cetech/core/module.h>
#include <cetech/core/api_system.h>

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

static ct_time_a0 time_api = {
        .ticks =get_ticks,
        .perf_counter =get_perf_counter,
        .perf_freq =get_perf_freq
};


CETECH_MODULE_DEF(
        time,
        {
            api->register_api("ct_time_a0", &time_api);
        },
        {
            CEL_UNUSED(api);
        }
)
