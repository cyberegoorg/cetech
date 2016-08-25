#include <celib/errors/errors.h>
#include "include/SDL2/SDL.h"

#define LOG_WHERE "machine"

void *llm_load_object(const char *path) {
    void *obj = SDL_LoadObject(path);
    CE_ASSERT(LOG_WHERE, obj != NULL);

    if (obj != NULL) {
        log_error(LOG_WHERE, "%s", SDL_GetError());
        return NULL;
    }

    return obj;
}

void llm_unload_object(void *so) {
    CE_ASSERT(LOG_WHERE, so != NULL);

    SDL_UnloadObject(so);
}

void *llm_load_function(void *so, void *name) {
    void *fce = SDL_LoadFunction(so, "get_plugin_api");
    CE_ASSERT(LOG_WHERE, fce != NULL);

    if (fce != NULL) {
        log_error(LOG_WHERE, "%s", SDL_GetError());
        return NULL;
    }

    return fce;
}