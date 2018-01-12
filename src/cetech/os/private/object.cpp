#include <dlfcn.h>
#include <cetech/api/api_system.h>
#include <cetech/os/errors.h>
#include <cetech/os/object.h>
#include <cetech/module/module.h>

#include "cetech/log/log.h"
#include "celib/macros.h"

CETECH_DECL_API(ct_log_a0);

#define LOG_WHERE "object"

void *load_object(const char *path) {
    char buffer[128];
    uint32_t it = sprintf(buffer, "%s", path);

#ifdef CETECH_LINUX
    it += sprintf(buffer+it, ".so");
    void *obj = dlmopen(LM_ID_NEWLM, buffer, RTLD_LOCAL | RTLD_NOW);
#endif

#ifdef CETECH_DARWIN
    it += sprintf(buffer+it, ".dylib");
    void *obj = dlopen(buffer, RTLD_LOCAL | RTLD_NOW);
#endif


    if (obj == NULL) {
        ct_log_a0.error(LOG_WHERE, "Could not load object file %s", dlerror());
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
    if(!so) {
        return NULL;
    }

    void *fce = dlsym(so, name);

    if (fce == NULL) {
        ct_log_a0.error(LOG_WHERE, "%s", dlerror());
        return NULL;
    }

    return fce;
}


static ct_object_a0 object_api = {
        .load  = load_object,
        .unload  = unload_object,
        .load_function  = load_function
};

CETECH_MODULE_DEF(
        object,
        {
            CETECH_GET_API(api, ct_log_a0);
        },
        {
            CEL_UNUSED(reload);
            api->register_api("ct_object_a0", &object_api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
        }
)
