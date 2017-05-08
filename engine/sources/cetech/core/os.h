//
// Created by ondra on 8.5.17.
//

#ifndef CETECH_OS_H
#define CETECH_OS_H

#include <stdint.h>

//==============================================================================
// CPU
//==============================================================================

enum {
    CPU_API_ID = 448453211
};

struct cpu_api_v0 {
    int (*count)();
};

enum {
    OBJECT_API_ID = 2223344
};


struct object_api_v0 {
    void *(*load)(const char *path);

    void (*unload)(void *so);

    void *(*load_function)(void *so,
                           void *name);
};


int exec(const char *argv);


enum {
    TIME_API_ID = 394952
};

//==============================================================================
// Time
// ==============================================================================

struct time_api_v0 {
    uint32_t (*get_ticks)();
    uint64_t (*get_perf_counter)();
    uint64_t (*get_perf_freq)();
};


#endif //CETECH_OS_H
