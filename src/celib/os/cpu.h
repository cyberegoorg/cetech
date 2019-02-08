//                          **OS cpu**
//

#ifndef CE_OS_CPU_H
#define CE_OS_CPU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#define CE_OS_CPU_API \
    CE_ID64_0("ce_os_cpu_a0", 0x41ff4437e99cbff7ULL)

struct ce_os_cpu_a0 {
    // Get cpu core count
    int (*count)();
};


CE_MODULE(ce_os_cpu_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_OS_CPU_H
