#include <celib/id.h>


#include <celib/macros.h>
#include "celib/log.h"
#include "celib/memory/allocator.h"
#include "celib/api.h"

#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_syswm.h"
#include <celib/os/window.h>

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


static struct {
    enum ce_window_flags from;
    SDL_WindowFlags to;
} _flag_to_sdl[] = {
        {.from = WINDOW_NOFLAG, .to =  0},
        {.from = WINDOW_FULLSCREEN, .to = SDL_WINDOW_FULLSCREEN},
        {.from = WINDOW_SHOWN, .to = SDL_WINDOW_SHOWN},
        {.from = WINDOW_HIDDEN, .to = SDL_WINDOW_HIDDEN},
        {.from = WINDOW_BORDERLESS, .to = SDL_WINDOW_BORDERLESS},
        {.from = WINDOW_RESIZABLE, .to = SDL_WINDOW_RESIZABLE},
        {.from = WINDOW_MINIMIZED, .to = SDL_WINDOW_MINIMIZED},
        {.from = WINDOW_MAXIMIZED, .to = SDL_WINDOW_MAXIMIZED},
        {.from = WINDOW_INPUT_GRABBED, .to = SDL_WINDOW_INPUT_GRABBED},
        {.from = WINDOW_INPUT_FOCUS, .to = SDL_WINDOW_INPUT_FOCUS},
        {.from = WINDOW_MOUSE_FOCUS, .to = SDL_WINDOW_MOUSE_FOCUS},
        {.from = WINDOW_FULLSCREEN_DESKTOP, .to =  SDL_WINDOW_FULLSCREEN_DESKTOP},
        {.from = WINDOW_ALLOW_HIGHDPI, .to = SDL_WINDOW_ALLOW_HIGHDPI},
        {.from = WINDOW_MOUSE_CAPTURE, .to = SDL_WINDOW_MOUSE_CAPTURE},
        {.from = WINDOW_ALWAYS_ON_TOP, .to = SDL_WINDOW_ALWAYS_ON_TOP},
        {.from = WINDOW_SKIP_TASKBAR, .to = SDL_WINDOW_SKIP_TASKBAR},
        {.from = WINDOW_UTILITY, .to = SDL_WINDOW_UTILITY},
        {.from = WINDOW_TOOLTIP, .to = SDL_WINDOW_TOOLTIP},
        {.from = WINDOW_POPUP_MENU, .to = SDL_WINDOW_POPUP_MENU},
};

static uint32_t _sdl_flags(uint32_t flags) {
    uint32_t sdl_flags = 0;

    for (uint32_t i = 1; i < CE_ARRAY_LEN(_flag_to_sdl); ++i) {
        if (flags & _flag_to_sdl[i].from) {
            sdl_flags |= _flag_to_sdl[i].to;
        }
    }

    return sdl_flags;
}

//==============================================================================
// Interface
//==============================================================================

void window_set_title(ce_window_o0 *w,
                      const char *title) {
    SDL_SetWindowTitle((SDL_Window *) w, title);
}

const char *window_get_title(ce_window_o0 *w) {
    return SDL_GetWindowTitle((SDL_Window *) w);
}

void window_resize(ce_window_o0 *w,
                   uint32_t width,
                   uint32_t height) {
    SDL_SetWindowSize((SDL_Window *) w, width, height);
}

void window_get_size(ce_window_o0 *window,
                     uint32_t *width,
                     uint32_t *height) {
    int w, h;
    w = h = 0;

    SDL_GetWindowSize((SDL_Window *) window, &w, &h);

    *width = (uint32_t) w;
    *height = (uint32_t) h;
}

void *window_native_window_ptr(ce_window_o0 *w) {
    SDL_SysWMinfo wmi = {};

    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo((SDL_Window *) w, &wmi)) {
        return 0;
    }

#if defined(CE_WINDOWS)
    return (void *) wmi.info.win.window;
#elif CE_PLATFORM_LINUX
    return (void *) wmi.info.x11.window;
#elif CE_PLATFORM_OSX
    return (void *) wmi.info.cocoa.window;
#endif
}

void *window_native_display_ptr(ce_window_o0 *w) {
    SDL_SysWMinfo wmi;

    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo((SDL_Window *) w, &wmi)) {
        return 0;
    }

#if defined(CE_WINDOWS)
    return (void *) wmi.info.win.hdc;
#elif CE_PLATFORM_LINUX
    return (void *) wmi.info.x11.display;
#elif CE_PLATFORM_OSX
    return (0);
#endif
}

void warp_mouse(ce_window_o0 *w,
                int x,
                int y) {
    SDL_WarpMouseInWindow((SDL_Window *) w, x, y);
}

struct ce_window_t0 *window_new(const char *title,
                                enum ce_window_pos x,
                                enum ce_window_pos y,
                                const int32_t width,
                                const int32_t height,
                                uint32_t flags,
                                ce_alloc_t0 *alloc) {

    struct ce_window_t0 *window = CE_ALLOC(alloc, ce_window_t0, sizeof(ce_window_t0));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window *w = SDL_CreateWindow(
            title,
            _sdl_pos(x), _sdl_pos(y),
            width, height,
            _sdl_flags(flags)
    );

    if (w == NULL) {
        ce_log_a0->error("sys", "Could not create window: %s",
                         SDL_GetError());
    }

    *window = (ce_window_t0) {
            .inst  = w,
            .set_title = window_set_title,
            .get_title = window_get_title,
            .resize = window_resize,
            .size = window_get_size,
            .native_window_ptr = window_native_window_ptr,
            .native_display_ptr = window_native_display_ptr,
            .warp_mouse = warp_mouse,
    };

    return window;
}


struct ce_window_t0 *window_new_from(void *hndl,
                                     ce_alloc_t0 *alloc) {

    struct ce_window_t0 *window = CE_ALLOC(alloc, ce_window_t0, sizeof(ce_window_t0));

    SDL_Window *w = SDL_CreateWindowFrom(hndl);

    if (w == NULL) {
        ce_log_a0->error("sys", "Could not create window: %s",
                         SDL_GetError());
    }

    *window = (ce_window_t0) {
            .inst  = w,
            .set_title = window_set_title,
            .get_title = window_get_title,
            .resize = window_resize,
            .size = window_get_size,
            .native_window_ptr = window_native_window_ptr,
            .native_display_ptr = window_native_display_ptr,
            .warp_mouse = warp_mouse,
    };

    return window;
}

void window_destroy(struct ce_window_t0 *w,
                    ce_alloc_t0 *alloc) {
    SDL_DestroyWindow((SDL_Window *) w->inst);
    CE_FREE(alloc, w);
}

struct ce_os_window_a0 window_api = {
        .create = window_new,
        .create_from = window_new_from,
        .destroy = window_destroy,
};


struct ce_os_window_a0 *ce_os_window_a0 = &window_api;

int sdl_window_init(struct ce_api_a0 *api) {
    return 1;
}

void sdl_window_shutdown() {

}
