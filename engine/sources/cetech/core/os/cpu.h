#ifndef CETECH_CPU_H
#define CETECH_CPU_H

#include <stdint.h>

//==============================================================================
// CPU
//==============================================================================

struct cpu_api_v0 {
    int (*count)();
};

#endif //CETECH_CPU_H
