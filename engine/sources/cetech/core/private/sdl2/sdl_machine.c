//==============================================================================
// Includes
//==============================================================================

#include <include/SDL2/SDL.h>

#include <cetech/core/array.inl>
#include <cetech/core/eventstream.inl>
#include <cetech/core/module.h>

#include <cetech/kernel/application.h>
#include <cetech/core/api.h>

#include "sdl_parts.h"

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

static struct app_api_v0 app_api_v0;

//==============================================================================
// Interface
//==============================================================================

int sdl_init( struct api_v0 *api) {
    _G = (struct G) {0};

    USE_API(api, app_api_v0);


    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        log_error(LOG_WHERE, "Could not init sdl - %s", SDL_GetError());
        return 0;
    }

    return 1;
}

void sdl_shutdown() {
    _G = (struct G) {0};

    SDL_Quit();
}

void sdl_process(struct eventstream *stream) {
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
                break;
        }
    }
}

