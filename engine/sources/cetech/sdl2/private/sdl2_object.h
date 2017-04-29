#include "cetech/os/errors.h"
#include "include/SDL2/SDL.h"

void *cel_load_object(const char *path) {
    void *obj = SDL_LoadObject(path);
    CEL_ASSERT("cebase", obj != NULL);

    if (obj == NULL) {
        log_error("cebase", "%s", SDL_GetError());
        return NULL;
    }

    return obj;
}

void cel_unload_object(void *so) {
    CEL_ASSERT("cebase", so != NULL);

    SDL_UnloadObject(so);
}

void *cel_load_function(void *so,
                        void *name) {
    void *fce = SDL_LoadFunction(so, "get_plugin_api");
    CEL_ASSERT("cebase", fce != NULL);

    if (fce == NULL) {
        log_error("cebase", "%s", SDL_GetError());
        return NULL;
    }

    return fce;
}