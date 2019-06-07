//                          **OS time**
//

#ifndef CE_OS_TIME_H
#define CE_OS_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#define CE_OS_TIME_API \
    CE_ID64_0("ce_os_time_a0", 0x909d08abc10715f8ULL)


struct ce_os_time_a0 {
    uint32_t (*ticks)();

    uint64_t (*perf_counter)();

    uint64_t (*perf_freq)();

    uint64_t (*timestamp_ms)();
};

CE_MODULE(ce_os_time_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_OS_TIME_H
