#include <dlfcn.h>

#include <cetech/kernel/errors.h>

void *load_object(const char *path) {
    void *obj = dlopen(path, RTLD_NOW|RTLD_LOCAL);

    CETECH_ASSERT("cebase", obj != NULL);

    if (obj == NULL) {
        log_api_v0.error("cebase", "%s", SDL_GetError());
        return NULL;
    }

    return obj;
}

void unload_object(void *so) {
    CETECH_ASSERT("cebase", so != NULL);

    dlclose(so);
}

void *load_function(void *so,
                    const char *name) {
    void *fce = dlsym(so, "load_module");
    CETECH_ASSERT("cebase", fce != NULL);

    if (fce == NULL) {
        log_api_v0.error("cebase", "%s", SDL_GetError());
        return NULL;
    }

    return fce;
}