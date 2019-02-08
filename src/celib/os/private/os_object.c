#include <celib/platform.h>
#include <celib/os/object.h>

#if CE_PLATFORM_LINUX
#define _GNU_SOURCE 0
#include <dlfcn.h>
#endif

#if CE_PLATFORM_OSX
#include <dlfcn.h>
#endif

#include <dlfcn.h>

#include <celib/api.h>

#include <celib/module.h>
#include <stdio.h>

#include "celib/log.h"

#include "celib/macros.h"


#define LOG_WHERE "object"

void *load_object(const char *path) {
    char buffer[128];
    uint32_t it = sprintf(buffer, "%s", path);

#if CE_PLATFORM_LINUX
    it += sprintf(buffer+it, ".so");
    void *obj = dlmopen(LM_ID_NEWLM, buffer, RTLD_GLOBAL | RTLD_NOW);
#endif

#if CE_PLATFORM_OSX
    it += sprintf(buffer + it, ".dylib");
    void *obj = dlopen(buffer, RTLD_GLOBAL | RTLD_NOW);
#endif

    if (obj == NULL) {
        ce_log_a0->error(LOG_WHERE, "Could not load object file %s", dlerror());
        return NULL;
    }

    return obj;
}

void unload_object(void *so) {
    CE_ASSERT(LOG_WHERE, so != NULL);

    dlclose(so);
}

void *load_function(void *so,
                    const char *name) {
    if (!so) {
        return NULL;
    }

    void *fce = dlsym(so, name);

    if (fce == NULL) {
        ce_log_a0->error(LOG_WHERE, "%s", dlerror());
        return NULL;
    }

    return fce;
}


struct ce_os_object_a0 object_api = {
        .load  = load_object,
        .unload  = unload_object,
        .load_function  = load_function
};

struct ce_os_object_a0 *ce_os_object_a0 = &object_api;
