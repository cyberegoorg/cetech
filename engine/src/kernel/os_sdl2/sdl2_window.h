#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_syswm.h"

#include <cetech/kernel/sdl2_os.h>
#include <cetech/kernel/log.h>

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

static uint32_t _sdl_flags(enum window_flags flags) {
    uint32_t sdl_flags = 0;

    if (flags & WINDOW_FULLSCREEN) {
        sdl_flags |= SDL_WINDOW_FULLSCREEN;
    }

    return sdl_flags;
}

//==============================================================================
// Interface
//==============================================================================

os_window_t *window_new(const char *title,
                        enum window_pos x,
                        enum window_pos y,
                        const int32_t width,
                        const int32_t height,
                        enum window_flags flags) {

    SDL_Window *w = SDL_CreateWindow(
            title,
            _sdl_pos(x), _sdl_pos(y),
            width, height,
            _sdl_flags(flags)
    );

    if (w == NULL) {
        log_api_v0.error("sys", "Could not create window: %s",
                         SDL_GetError());
    }

    return (os_window_t *) w;
}

os_window_t *window_new_from(void *hndl) {
    SDL_Window *w = SDL_CreateWindowFrom(hndl);

    if (w == NULL) {
        log_api_v0.error("sys", "Could not create window: %s",
                         SDL_GetError());
    }

    return (os_window_t *) w;
}

void window_destroy(os_window_t *w) {
    SDL_DestroyWindow((SDL_Window *) w);
}

void window_set_title(os_window_t *w,
                      const char *title) {
    SDL_SetWindowTitle((SDL_Window *) w, title);
}

const char *window_get_title(os_window_t *w) {
    return SDL_GetWindowTitle((SDL_Window *) w);
}

void window_update(os_window_t *w) {
    SDL_UpdateWindowSurface((SDL_Window *) w);
}

void window_resize(os_window_t *w,
                   uint32_t width,
                   uint32_t height) {
    SDL_SetWindowSize((SDL_Window *) w, width, height);
}

void window_get_size(os_window_t *window,
                     uint32_t *width,
                     uint32_t *height) {
    int w, h;
    w = h = 0;

    SDL_GetWindowSize((SDL_Window *) window, &w, &h);

    *width = (uint32_t) w;
    *height = (uint32_t) h;
}

void *window_native_window_ptr(os_window_t *w) {
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

void *window_native_display_ptr(os_window_t *w) {
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
