#include <dlfcn.h>

#include <cetech/kernel/errors.h>

void *load_object(const char *path) {
    void *obj = dlopen(path, RTLD_NOW|RTLD_LOCAL);

    CETECH_ASSERT("os_object", obj != NULL);

    if (obj == NULL) {
        ct_log_a0.error("os_object", "%s", dlerror());
        return NULL;
    }

    return obj;
}

void unload_object(void *so) {
    CETECH_ASSERT("os_object", so != NULL);

    dlclose(so);
}

void *load_function(void *so,
                    const char *name) {
    void *fce = dlsym(so, "load_module");
    CETECH_ASSERT("os_object", fce != NULL);

    if (fce == NULL) {
        ct_log_a0.error("os_object", "%s", dlerror());
        return NULL;
    }

    return fce;
}