#ifndef CETECH_TIME_H
#define CETECH_TIME_H

#include <stdint.h>

struct time_api_v0 {
    uint32_t (*ticks)();

    uint64_t (*perf_counter)();

    uint64_t (*perf_freq)();
};

#endif //CETECH_TIME_H
