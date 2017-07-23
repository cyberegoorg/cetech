#include <dlfcn.h>

#include <cetech/os/errors.h>
#include <cetech/api_system.h>
#include <cetech/log.h>
#include <cetech/machine/machine.h>
#include <cetech/os/object.h>

CETECH_DECL_API(ct_log_a0);

#define LOG_WHERE "object"

void *load_object(const char *path) {
    void *obj = dlopen(path, RTLD_NOW | RTLD_LOCAL);

    CETECH_ASSERT(LOG_WHERE, obj != NULL);

    if (obj == NULL) {
        ct_log_a0.error(LOG_WHERE, "%s", dlerror());
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
    void *fce = dlsym(so, "load_module");

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

extern "C" void object_load_module(ct_api_a0 *api) {
    CETECH_GET_API(api, ct_log_a0);

    api->register_api("ct_object_a0", &object_api);
}

extern "C" void object_unload_module(ct_api_a0 *api) {

}