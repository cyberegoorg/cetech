#include <stdint.h>
#include <cetech/machine/types.h>
#include <include/SDL2/SDL.h>
#include <include/SDL2/SDL_syswm.h>

#define LOG_WHERE "windowsys"

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

static uint32_t _sdl_flags(enum WindowFlags flags) {
    uint32_t sdl_flags = 0;

    if (flags & WINDOW_FULLSCREEN) {
        sdl_flags |= SDL_WINDOW_FULLSCREEN;
    }

    return sdl_flags;
}


//==============================================================================
// Interface
//==============================================================================

int sdl_window_init() {
    log_debug(LOG_WHERE, "Init");

    return 1;
}

void sdl_window_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");
}

void sdl_window_process() {
}

window_t machine_window_new(const char *title, enum WindowPos x, enum WindowPos y, const i32 width, const i32 height,
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

window_t machine_window_new_from(void *hndl) {
    SDL_Window *w = SDL_CreateWindowFrom(hndl);

    if (w == NULL) {
        log_error("sys", "Could not create window: %s", SDL_GetError());
    }

    return (window_t) {.w = w};
}

void machine_window_destroy(window_t w) {
    SDL_DestroyWindow(w.w);
}

void machine_window_set_title(window_t w, const char *title) {
    SDL_SetWindowTitle(w.w, title);
}

const char *machine_window_get_title(window_t w) {
    return SDL_GetWindowTitle(w.w);
}

void machine_window_update(window_t w) {
    SDL_UpdateWindowSurface(w.w);
}

void machine_window_resize(window_t w, uint32_t width, uint32_t height) {
    SDL_SetWindowSize(w.w, width, height);
}

void* machine_window_native_window_ptr(window_t w) {
    SDL_SysWMinfo wmi;

    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo(w.w, &wmi)) {
        return 0;
    }

#if CETECH_WINDOWS
    return (void *) wmi.info.win.window;
#elif CETECH_LINUX
    return (void *) wmi.info.x11.window;
#elif sCETECH_DARWIN
    return (void *) wmi.info.cocoa.window;
#endif
}

void* machine_window_native_display_ptr(window_t w) {
    SDL_SysWMinfo wmi;

    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo(w.w, &wmi)) {
        return 0;
    }

#if CETECH_WINDOWS
    return (void *) wmi.info.win.hdc;
#elif CETECH_LINUX
    return (void *) wmi.info.x11.display;
#elif sCETECH_DARWIN
    return (0;
#endif
}
