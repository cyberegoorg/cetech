#ifndef CETECH_TIME_H
#define CETECH_TIME_H

#include <stdint.h>

struct time_api_v0 {
    uint32_t (*get_ticks)();

    uint64_t (*get_perf_counter)();

    uint64_t (*get_perf_freq)();
};

#endif //CETECH_TIME_H
