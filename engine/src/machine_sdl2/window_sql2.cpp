#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_syswm.h"

#include <cetech/machine/machine.h>
#include <cetech/core/log.h>
#include <cetech/core/api_system.h>

CETECH_DECL_API(ct_log_a0);

//==============================================================================
// Private
//==============================================================================

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

static uint32_t _sdl_flags(enum ct_window_flags flags) {
    uint32_t sdl_flags = 0;

    if (flags & WINDOW_FULLSCREEN) {
        sdl_flags |= SDL_WINDOW_FULLSCREEN;
    }

    return sdl_flags;
}

//==============================================================================
// Interface
//==============================================================================

ct_window_t *window_new(const char *title,
                        enum ct_window_pos x,
                        enum ct_window_pos y,
                        const int32_t width,
                        const int32_t height,
                        enum ct_window_flags flags) {

    SDL_Window *w = SDL_CreateWindow(
            title,
            _sdl_pos(x), _sdl_pos(y),
            width, height,
            _sdl_flags(flags)
    );

    if (w == NULL) {
        ct_log_a0.error("sys", "Could not create window: %s",
                        SDL_GetError());
    }

    return (ct_window_t *) w;
}

ct_window_t *window_new_from(void *hndl) {
    SDL_Window *w = SDL_CreateWindowFrom(hndl);

    if (w == NULL) {
        ct_log_a0.error("sys", "Could not create window: %s",
                        SDL_GetError());
    }

    return (ct_window_t *) w;
}

void window_destroy(ct_window_t *w) {
    SDL_DestroyWindow((SDL_Window *) w);
}

void window_set_title(ct_window_t *w,
                      const char *title) {
    SDL_SetWindowTitle((SDL_Window *) w, title);
}

const char *window_get_title(ct_window_t *w) {
    return SDL_GetWindowTitle((SDL_Window *) w);
}

void window_update(ct_window_t *w) {
    SDL_UpdateWindowSurface((SDL_Window *) w);
}

void window_resize(ct_window_t *w,
                   uint32_t width,
                   uint32_t height) {
    SDL_SetWindowSize((SDL_Window *) w, width, height);
}

void window_get_size(ct_window_t *window,
                     uint32_t *width,
                     uint32_t *height) {
    int w, h;
    w = h = 0;

    SDL_GetWindowSize((SDL_Window *) window, &w, &h);

    *width = (uint32_t) w;
    *height = (uint32_t) h;
}

void *window_native_window_ptr(ct_window_t *w) {
    SDL_SysWMinfo wmi;

    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo((SDL_Window *) w, &wmi)) {
        return 0;
    }

#if CETECH_WINDOWS
    return (void *) wmi.info.win.window;
#elif CETECH_LINUX
    return (void *) wmi.info.x11.window;
#elif CETECH_DARWIN
    return (void *) wmi.info.cocoa.window;
#endif
}

void *window_native_display_ptr(ct_window_t *w) {
    SDL_SysWMinfo wmi;

    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo((SDL_Window *) w, &wmi)) {
        return 0;
    }

#if CETECH_WINDOWS
    return (void *) wmi.info.win.hdc;
#elif CETECH_LINUX
    return (void *) wmi.info.x11.display;
#elif CETECH_DARWIN
    return (0);
#endif
}


static ct_window_a0 window_api = {
        .create = window_new,
        .create_from = window_new_from,
        .destroy = window_destroy,
        .set_title = window_set_title,
        .get_title = window_get_title,
        .update = window_update,
        .resize = window_resize,
        .size = window_get_size,
        .native_window_ptr = window_native_window_ptr,
        .native_display_ptr = window_native_display_ptr
};


int sdl_window_init(ct_api_a0 *api) {
    CETECH_GET_API(api, ct_log_a0);
    api->register_api("ct_window_a0", &window_api);
    return 1;
}

void sdl_window_shutdown() {

}
