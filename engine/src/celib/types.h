#pragma once

#include <stdint.h>

#ifdef __MACH__
    #define thread_local __thread
#endif
