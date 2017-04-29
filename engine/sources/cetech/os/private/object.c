#include "cetech/os/errors.h"

#if defined(CELIB_USE_SDL)

#include "include/SDL2/SDL.h"

#endif

void *cel_load_object(const char *path) {
#if defined(CELIB_USE_SDL)
    void *obj = SDL_LoadObject(path);
    CEL_ASSERT("cebase", obj != NULL);

    if (obj == NULL) {
        log_error("cebase", "%s", SDL_GetError());
        return NULL;
    }

    return obj;
#endif
}

void cel_unload_object(void *so) {
#if defined(CELIB_USE_SDL)

    CEL_ASSERT("cebase", so != NULL);

    SDL_UnloadObject(so);
#endif
}

void *cel_load_function(void *so,
                        void *name) {
#if defined(CELIB_USE_SDL)

    void *fce = SDL_LoadFunction(so, "get_plugin_api");
    CEL_ASSERT("cebase", fce != NULL);

    if (fce == NULL) {
        log_error("cebase", "%s", SDL_GetError());
        return NULL;
    }

    return fce;
#endif
}