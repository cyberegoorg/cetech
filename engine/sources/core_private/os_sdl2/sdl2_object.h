#include "include/SDL2/SDL.h"

#include <cetech/core/errors.h>

void *load_object(const char *path) {
    void *obj = SDL_LoadObject(path);
    CETECH_ASSERT("cebase", obj != NULL);

    if (obj == NULL) {
        log_api_v0.error("cebase", "%s", SDL_GetError());
        return NULL;
    }

    return obj;
}

void unload_object(void *so) {
    CETECH_ASSERT("cebase", so != NULL);

    SDL_UnloadObject(so);
}

void *load_function(void *so,
                    void *name) {
    void *fce = SDL_LoadFunction(so, "get_plugin_api");
    CETECH_ASSERT("cebase", fce != NULL);

    if (fce == NULL) {
        log_api_v0.error("cebase", "%s", SDL_GetError());
        return NULL;
    }

    return fce;
}