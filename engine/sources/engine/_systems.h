#ifdef _SYSTEM_DECL
#define SYSTEM(n) int n##_init(); void n##_shutdown();
#else
#define SYSTEM(n) {.name= #n, .init=n##_init, .shutdown=n##_shutdown},

static const struct {
    const char *name;

    int (*init)();

    void (*shutdown)();
} _SYSTEMS[] = {
#endif

#include "static_systems.h"

#ifndef _SYSTEM_DECL
};
#endif
#undef SYSTEM