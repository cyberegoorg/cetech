#ifndef CETECH_TIME_H
#define CETECH_TIME_H


#ifdef __cplusplus
extern "C" {
#endif


//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <stddef.h>


//==============================================================================
// Time
//==============================================================================

struct ct_time_a0 {
    uint32_t (*ticks)();

    uint64_t (*perf_counter)();

    uint64_t (*perf_freq)();
};


#ifdef __cplusplus
}
#endif

#endif //CETECH_TIME_H
