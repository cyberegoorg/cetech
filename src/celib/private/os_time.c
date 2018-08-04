#include <celib/os.h>
#include <celib/module.h>
#include <celib/api_system.h>
#include "include/SDL2/SDL.h"
#include "celib/macros.h"

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

struct ce_os_time_a0 time_api = {
        .ticks =get_ticks,
        .perf_counter =get_perf_counter,
        .perf_freq =get_perf_freq
};

struct ce_os_time_a0 *ct_time_a0 = &time_api;

