#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_syswm.h"

#include "cetech/kernel/log.h"
#include "cetech/kernel/api_system.h"

#include "cetech/modules/machine/machine.h"
#include "cetech/modules/machine/window.h"

#include "celib/allocator.h"

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


static struct {
    enum ct_window_flags from;
    SDL_WindowFlags to;
} _flag_to_sdl[] = {
        {.from = WINDOW_NOFLAG, .to =  SDL_WindowFlags(0)},
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

    for (uint32_t i = 1; i < CETECH_ARRAY_LEN(_flag_to_sdl); ++i) {
        if (flags & _flag_to_sdl[i].from) {
            sdl_flags |= _flag_to_sdl[i].to;
        }
    }

    return sdl_flags;
}

//==============================================================================
// Interface
//==============================================================================

void window_set_title(ct_window_ints *w,
                      const char *title) {
    SDL_SetWindowTitle((SDL_Window *) w, title);
}

const char *window_get_title(ct_window_ints *w) {
    return SDL_GetWindowTitle((SDL_Window *) w);
}

void window_update(ct_window_ints *w) {
    SDL_UpdateWindowSurface((SDL_Window *) w);
}

void window_resize(ct_window_ints *w,
                   uint32_t width,
                   uint32_t height) {
    SDL_SetWindowSize((SDL_Window *) w, width, height);
}

void window_get_size(ct_window_ints *window,
                     uint32_t *width,
                     uint32_t *height) {
    int w, h;
    w = h = 0;

    SDL_GetWindowSize((SDL_Window *) window, &w, &h);

    *width = (uint32_t) w;
    *height = (uint32_t) h;
}

void *window_native_window_ptr(ct_window_ints *w) {
    SDL_SysWMinfo wmi;

    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo((SDL_Window *) w, &wmi)) {
        return 0;
    }

#if defined(CETECH_WINDOWS)
    return (void *) wmi.info.win.window;
#elif defined(CETECH_LINUX)
    return (void *) wmi.info.x11.window;
#elif defined(CETECH_DARWIN)
    return (void *) wmi.info.cocoa.window;
#endif
}

void *window_native_display_ptr(ct_window_ints *w) {
    SDL_SysWMinfo wmi;

    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo((SDL_Window *) w, &wmi)) {
        return 0;
    }

#if defined(CETECH_WINDOWS)
    return (void *) wmi.info.win.hdc;
#elif defined(CETECH_LINUX)
    return (void *) wmi.info.x11.display;
#elif defined(CETECH_DARWIN)
    return (0);
#endif
}

ct_window *window_new(struct cel_alloc *alloc,
                      const char *title,
                      enum ct_window_pos x,
                      enum ct_window_pos y,
                      const int32_t width,
                      const int32_t height,
                      uint32_t flags) {

    auto *window = CEL_ALLOCATE(alloc, ct_window, sizeof(ct_window));

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

    *window = (ct_window) {
            .inst  = w,
            .set_title = window_set_title,
            .get_title = window_get_title,
            .update = window_update,
            .resize = window_resize,
            .size = window_get_size,
            .native_window_ptr = window_native_window_ptr,
            .native_display_ptr = window_native_display_ptr
    };

    return window;
}

ct_window *window_new_from(struct cel_alloc *alloc,
                           void *hndl) {
    auto *window = CEL_ALLOCATE(alloc, ct_window, sizeof(ct_window));

    SDL_Window *w = SDL_CreateWindowFrom(hndl);

    if (w == NULL) {
        ct_log_a0.error("sys", "Could not create window: %s",
                        SDL_GetError());
    }

    *window = (ct_window) {
            .inst  = w,
            .set_title = window_set_title,
            .get_title = window_get_title,
            .update = window_update,
            .resize = window_resize,
            .size = window_get_size,
            .native_window_ptr = window_native_window_ptr,
            .native_display_ptr = window_native_display_ptr
    };

    return window;
}

void window_destroy(cel_alloc *alloc,
                    ct_window *w) {
    SDL_DestroyWindow((SDL_Window *) w->inst);
    CEL_FREE(alloc, w);
}

static ct_window_a0 window_api = {
        .create = window_new,
        .create_from = window_new_from,
        .destroy = window_destroy,
};


int sdl_window_init(ct_api_a0 *api) {
    CETECH_GET_API(api, ct_log_a0);
    api->register_api("ct_window_a0", &window_api);
    return 1;
}

void sdl_window_shutdown() {

}
