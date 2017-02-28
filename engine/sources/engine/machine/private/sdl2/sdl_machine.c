//==============================================================================
// Includes
//==============================================================================

#include <include/SDL2/SDL.h>
#include "celib/containers/array.h"
#include "celib/containers/eventstream.h"

#include <engine/plugin/plugin_api.h>
#include "engine/application/application.h"

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

static struct ApplicationApiV0 ApplicationApiV0;

//==============================================================================
// Interface
//==============================================================================

int sdl_init(get_api_fce_t get_engine_api) {
    _G = (struct G) {0};

    ApplicationApiV0 = *(struct ApplicationApiV0 *) get_engine_api(
            APPLICATION_API_ID, 0);

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
                ApplicationApiV0.quit();
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

