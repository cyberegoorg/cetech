#include <celib/module.h>
#include <celib/api.h>
#include "celib/macros.h"
#include <celib/os/time.h>

#include "include/SDL2/SDL.h"

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

uint64_t timestamp_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    time_t s = spec.tv_sec;
    uint64_t ms = round(spec.tv_nsec / 1.0e6);
    if (ms > 999) {
        s++;
        ms = 0;
    }

    ms += s * 1000;

    return ms;
}

struct ce_os_time_a0 time_api = {
        .ticks =get_ticks,
        .perf_counter = get_perf_counter,
        .perf_freq = get_perf_freq,
        .timestamp_ms = timestamp_ms,
};

struct ce_os_time_a0 *ce_os_time_a0 = &time_api;

