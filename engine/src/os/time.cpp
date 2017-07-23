#include "include/SDL2/SDL.h"
#include <cetech/macros.h>
#include <cetech/machine/machine.h>
#include <cetech/api_system.h>
#include <cetech/os/time.h>

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

extern "C" void time_load_module(ct_api_a0 *api) {
    api->register_api("ct_time_a0", &time_api);
}

extern "C" void time_unload_module(ct_api_a0 *api) {
}
