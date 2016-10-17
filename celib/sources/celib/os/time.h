#ifndef CETECH_TIME_H
#define CETECH_TIME_H

#include "../types.h"

//==============================================================================
// Time
// ==============================================================================


u32 os_get_ticks();

u64 os_get_perf_counter();
u64 os_get_perf_freq();

#endif //CETECH_TIME_H
