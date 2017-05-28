//==============================================================================
// Includes
//==============================================================================

#include <include/SDL2/SDL.h>

#include <cetech/core/container/array.inl>
#include <cetech/core/container/eventstream.inl>
#include <cetech/core/module.h>

#include <cetech/core/application.h>
#include <cetech/core/api.h>

//==============================================================================
// Keyboard part
//==============================================================================

extern int sdl_keyboard_init(struct api_v0 *api);

extern void sdl_keyboard_shutdown();

extern void sdl_keyboard_process(struct eventstream *stream);


//==============================================================================
// Mouse part
//==============================================================================

extern int sdl_mouse_init(struct api_v0 *api);

extern void sdl_mouse_shutdown();

extern void sdl_mouse_process(struct eventstream *stream);

//==============================================================================
// Gamepad part
//==============================================================================

extern int sdl_gamepad_init(struct api_v0 *api);

extern void sdl_gamepad_shutdown();

extern void sdl_gamepad_process(struct eventstream *stream);

extern void sdl_gamepad_process_event(SDL_Event *event,
                                      struct eventstream *stream);

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "machine.sdl"

//==============================================================================
// Globals
//==============================================================================

static struct G {
    int _;
} _G = {0};

IMPORT_API(app_api_v0);
IMPORT_API(log_api_v0);


//==============================================================================
// Interface
//==============================================================================

int _machine_init_impl(struct api_v0 *api) {
    _G = (struct G) {0};

    GET_API(api, app_api_v0);
    GET_API(api, log_api_v0);


    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        log_api_v0.log_error(LOG_WHERE, "Could not init sdl - %s",
                             SDL_GetError());
        return 0;
    }

    sdl_gamepad_init(api);
    sdl_mouse_init(api);
    sdl_keyboard_init(api);

    return 1;
}

void _machine_shutdown_impl() {
    sdl_gamepad_shutdown();
    sdl_mouse_shutdown();
    sdl_keyboard_shutdown();

    SDL_Quit();

    _G = (struct G) {0};
}

void _machine_process_impl(struct eventstream *stream) {
    SDL_Event e;

    while (SDL_PollEvent(&e) > 0) {
        switch (e.type) {
            case SDL_QUIT:
                app_api_v0.quit();
                break;

            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
                sdl_gamepad_process_event(&e, stream);
                break;

            default:
                sdl_gamepad_process(stream);
                sdl_mouse_process(stream);
                sdl_keyboard_process(stream);
                break;
        }
    }
}

