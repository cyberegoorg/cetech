#ifndef CETECH_TIME_H
#define CETECH_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct time_api_v0 {
    uint32_t (*ticks)();

    uint64_t (*perf_counter)();

    uint64_t (*perf_freq)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_TIME_H
