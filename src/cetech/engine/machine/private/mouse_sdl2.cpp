//==============================================================================
// Incldues
//==============================================================================

#include <include/SDL2/SDL.h>

#include "celib/eventstream.inl"
#include "cetech/engine/machine/machine.h"
#include "cetech/engine/machine/window.h"

#include <cetech/application/application.h>
#include "cetech/engine/resource/resource.h"
#include "cetech/core/api/api_system.h"

using namespace celib;

//==============================================================================
// Defines
//==============================================================================

#define is_button_down(now, last) ((now) && !(last))
#define is_button_up(now, last)   (!(now) && (last))


//==============================================================================
// Globals
//==============================================================================

static struct G {
    uint8_t state[MOUSE_BTN_MAX];
    float position[2];
} _G = {};


CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_window_a0);

//==============================================================================
// Interface
//==============================================================================

int sdl_mouse_init(ct_api_a0 *api) {
    _G = (struct G) {};

    CETECH_GET_API(api, ct_app_a0);
    CETECH_GET_API(api, ct_window_a0);

    return 1;
}

void sdl_mouse_shutdown() {
    _G = (struct G) {};
}

void sdl_mouse_process(EventStream &stream) {
    int pos[2] = {};

    uint32_t state = SDL_GetMouseState(&pos[0], &pos[1]);

    uint8_t curent_state[MOUSE_BTN_MAX] = {};

    curent_state[MOUSE_BTN_LEFT] = (uint8_t) (state & SDL_BUTTON_LMASK);
    curent_state[MOUSE_BTN_RIGHT] = (uint8_t) (state & SDL_BUTTON_RMASK);
    curent_state[MOUSE_BTN_MIDLE] = (uint8_t) (state & SDL_BUTTON_MMASK);

    if ((pos[0] != _G.position[0]) || (pos[1] != _G.position[1])) {
        auto *main_window = ct_app_a0.main_window();

        uint32_t window_size[2] = {};
        main_window->size(main_window->inst, &window_size[0], &window_size[1]);

        _G.position[0] = pos[0];
        _G.position[1] = window_size[1] - pos[1];

        ct_mouse_move_event event;
        event.pos[0] = pos[0];
        event.pos[1] = window_size[1] - pos[1];

        eventstream::push(stream, EVENT_MOUSE_MOVE, event);
    }

    for (uint32_t i = 0; i < MOUSE_BTN_MAX; ++i) {
        ct_mouse_event event;
        event.button = i;

        if (is_button_down(curent_state[i], _G.state[i]))
            eventstream::push(stream, EVENT_MOUSE_DOWN, event);

        else if (is_button_up(curent_state[i], _G.state[i]))
            eventstream::push(stream, EVENT_MOUSE_UP, event);

        _G.state[i] = curent_state[i];
    }
}
