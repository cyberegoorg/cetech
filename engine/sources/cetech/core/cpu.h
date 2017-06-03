#ifndef CETECH_CPU_H
#define CETECH_CPU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

//==============================================================================
// CPU
//==============================================================================

struct cpu_api_v0 {
    int (*count)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_CPU_H
