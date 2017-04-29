#ifndef CELIB_TIME_H
#define CELIB_TIME_H

#include <stdint.h>

enum {
    TIME_API_ID = 394952
};

//==============================================================================
// Time
// ==============================================================================

struct time_api_v0 {
    uint32_t (*get_ticks)();
    uint64_t (*get_perf_counter)();
    uint64_t (*get_perf_freq)();
};

#endif //CELIB_TIME_H
