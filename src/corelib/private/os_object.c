#include <corelib/platform.h>

#if CT_PLATFORM_LINUX
#define _GNU_SOURCE 0
#include <dlfcn.h>
#endif

#if CT_PLATFORM_OSX
#include <dlfcn.h>
#endif

#include <dlfcn.h>

#include <corelib/api_system.h>
#include <corelib/os.h>
#include <corelib/module.h>
#include <stdio.h>

#include "corelib/log.h"

#include "corelib/macros.h"


#define LOG_WHERE "object"

void *load_object(const char *path) {
    char buffer[128];
    uint32_t it = sprintf(buffer, "%s", path);

#if CT_PLATFORM_LINUX
    it += sprintf(buffer+it, ".so");
    void *obj = dlmopen(LM_ID_NEWLM, buffer, RTLD_GLOBAL | RTLD_NOW);
#endif

#if CT_PLATFORM_OSX
    it += sprintf(buffer + it, ".dylib");
    void *obj = dlopen(buffer, RTLD_GLOBAL | RTLD_NOW);
#endif

    if (obj == NULL) {
        ct_log_a0->error(LOG_WHERE, "Could not load object file %s", dlerror());
        return NULL;
    }

    return obj;
}

void unload_object(void *so) {
    CETECH_ASSERT(LOG_WHERE, so != NULL);

    dlclose(so);
}

void *load_function(void *so,
                    const char *name) {
    if (!so) {
        return NULL;
    }

    void *fce = dlsym(so, name);

    if (fce == NULL) {
        ct_log_a0->error(LOG_WHERE, "%s", dlerror());
        return NULL;
    }

    return fce;
}


struct ct_object_a0 object_api = {
        .load  = load_object,
        .unload  = unload_object,
        .load_function  = load_function
};

struct ct_object_a0 *ct_object_a0 = &object_api;
