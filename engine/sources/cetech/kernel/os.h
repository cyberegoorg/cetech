//
// Created by ondra on 8.5.17.
//

#ifndef CETECH_OS_H
#define CETECH_OS_H

#include <stdint.h>

//==============================================================================
// CPU
//==============================================================================


struct cpu_api_v0 {
    int (*count)();
};


struct object_api_v0 {
    void *(*load)(const char *path);

    void (*unload)(void *so);

    void *(*load_function)(void *so,
                           void *name);
};


int exec(const char *argv);

//==============================================================================
// Time
// ==============================================================================

struct time_api_v0 {
    uint32_t (*get_ticks)();

    uint64_t (*get_perf_counter)();

    uint64_t (*get_perf_freq)();
};


#endif //CETECH_OS_H
