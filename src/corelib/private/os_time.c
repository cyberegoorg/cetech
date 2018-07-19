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

struct ct_os_time_a0 time_api = {
        .ticks =get_ticks,
        .perf_counter =get_perf_counter,
        .perf_freq =get_perf_freq
};

struct ct_os_time_a0 *ct_time_a0 = &time_api;

