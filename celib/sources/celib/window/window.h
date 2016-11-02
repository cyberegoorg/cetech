#ifndef CELIB_WINDOW_H
#define CELIB_WINDOW_H

#include "./types.h"
#include "../types.h"
#include "../log/log.h"

#if defined(CELIB_USE_SDL)

#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_syswm.h"

#endif

//==============================================================================
// Window interface
//==============================================================================

//==============================================================================
// Private
//==============================================================================

#if defined(CELIB_USE_SDL)

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

static uint32_t _sdl_flags(enum cel_window_flags flags) {
    uint32_t sdl_flags = 0;

    if (flags & WINDOW_FULLSCREEN) {
        sdl_flags |= SDL_WINDOW_FULLSCREEN;
    }

    return sdl_flags;
}

#endif

//==============================================================================
// Interface
//==============================================================================

static cel_window_t cel_window_new(const char *title,
                                   enum cel_window_pos x,
                                   enum cel_window_pos y,
                                   const i32 width,
                                   const i32 height,
                                   enum cel_window_flags flags) {
#if defined(CELIB_USE_SDL)
    SDL_Window *w = SDL_CreateWindow(
            title,
            _sdl_pos(x), _sdl_pos(y),
            width, height,
            _sdl_flags(flags)
    );

    if (w == NULL) {
        log_error("sys", "Could not create window: %s", SDL_GetError());
    }

    return (cel_window_t) {.w = w};
#endif
}

static cel_window_t cel_window_new_from(void *hndl) {
#if defined(CELIB_USE_SDL)

    SDL_Window *w = SDL_CreateWindowFrom(hndl);

    if (w == NULL) {
        log_error("sys", "Could not create window: %s", SDL_GetError());
    }

    return (cel_window_t) {.w = w};
#endif
}

static void cel_window_destroy(cel_window_t w) {
#if defined(CELIB_USE_SDL)

    SDL_DestroyWindow(w.w);
#endif
}

static void cel_window_set_title(cel_window_t w,
                                 const char *title) {
#if defined(CELIB_USE_SDL)

    SDL_SetWindowTitle(w.w, title);
#endif
}

static const char *cel_window_get_title(cel_window_t w) {
#if defined(CELIB_USE_SDL)

    return SDL_GetWindowTitle(w.w);
#endif
}

static void cel_window_update(cel_window_t w) {
#if defined(CELIB_USE_SDL)

    SDL_UpdateWindowSurface(w.w);
#endif
}

static void cel_window_resize(cel_window_t w,
                              uint32_t width,
                              uint32_t height) {
#if defined(CELIB_USE_SDL)

    SDL_SetWindowSize(w.w, width, height);
#endif
}

static void cel_window_get_size(cel_window_t window,
                                uint32_t *width,
                                uint32_t *height) {
#if defined(CELIB_USE_SDL)

    int w, h;
    w = h = 0;

    SDL_GetWindowSize(window.w, &w, &h);

    *width = (uint32_t) w;
    *height = (uint32_t) h;
#endif
}

static void *cel_window_native_cel_window_ptr(cel_window_t w) {
#if defined(CELIB_USE_SDL)

    SDL_SysWMinfo wmi;

    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo(w.w, &wmi)) {
        return 0;
    }

#if CELIB_WINDOWS
    return (void *) wmi.info.win.window;
#elif CELIB_LINUX
    return (void *) wmi.info.x11.window;
#elif CELIB_DARWIN
    return (void *) wmi.info.cocoa.window;
#endif
#endif
}

static void *cel_window_native_display_ptr(cel_window_t w) {
#if defined(CELIB_USE_SDL)

    SDL_SysWMinfo wmi;

    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo(w.w, &wmi)) {
        return 0;
    }

#if CELIB_WINDOWS
    return (void *) wmi.info.win.hdc;
#elif CELIB_LINUX
    return (void *) wmi.info.x11.display;
#elif CELIB_DARWIN
    return (0);
#endif
#endif
}

#endif //CELIB_WINDOW_H
