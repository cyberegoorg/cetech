
#include <cetech/windowsystem/windowsystem.h>
#include <celib/log/log.h>
#include "include/SDL2/SDL.h"

#define LOG_WHERE "windowsys"

static uint32_t _sdl_pos(const uint32_t pos) {
    switch (pos) {
        case WINDOWPOS_CENTERED:
            return SDL_WINDOWPOS_CENTERED;

        case WINDOWPOS_UNDEFINED:
            return SDL_WINDOWPOS_UNDEFINED;

        default:
            return pos;
    }
}

static uint32_t _sdl_flags(enum WindowFlags flags) {
    uint32_t sdl_flags = 0;

    if (flags & WINDOW_FULLSCREEN) {
        sdl_flags |= SDL_WINDOW_FULLSCREEN;
    }

    return sdl_flags;
}

int windowsys_init() {
    log_debug(LOG_WHERE, "Init");

    return 1;
}

void windowsys_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");
}

window_t windowsys_new_window(const char *title, enum WindowPos x, enum WindowPos y, const i32 width, const i32 height,
                              enum WindowFlags flags) {

    SDL_Window *w = SDL_CreateWindow(
            title,
            _sdl_pos(x), _sdl_pos(y),
            width, height,
            _sdl_flags(flags)
    );

    if (w == NULL) {
        log_error("sys", "Could not create window: %s", SDL_GetError());
    }

    return (window_t) {.w = w};
}

window_t windowsys_new_from(void *hndl) {
    SDL_Window *w = SDL_CreateWindowFrom(hndl);

    if (w == NULL) {
        log_error("sys", "Could not create window: %s", SDL_GetError());
    }

    return (window_t) {.w = w};
}

void windowsys_destroy_window(window_t w) {
    SDL_DestroyWindow(w.w);
}

void windowsys_set_title(window_t w, const char *title) {
    SDL_SetWindowTitle(w.w, title);
}

const char *windowsys_get_title(window_t w) {
    return SDL_GetWindowTitle(w.w);
}

void windowsys_update(window_t w) {
    SDL_UpdateWindowSurface(w.w);
}

void windowsys_resize(window_t w, uint32_t width, uint32_t height) {
    SDL_SetWindowSize(w.w, width, height);
}
