//==============================================================================
// Includes
//==============================================================================

#include <include/SDL2/SDL.h>

#include "cetech/core/eventstream.inl"
#include "cetech/engine/machine/machine.h"
#include "cetech/core/api_system.h"

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
    uint8_t state[KEY_MAX];
} _G = {};


//==============================================================================
// Interface
//==============================================================================

int sdl_keyboard_init(ct_api_a0 *api) {
    CT_UNUSED(api);
    _G = (struct G) {};

    return 1;
}

void sdl_keyboard_shutdown() {
    _G = (struct G) {};
}

void sdl_keyboard_process(EventStream &stream) {
    const uint8_t *state = SDL_GetKeyboardState(NULL);
    ct_keyboard_event keyboard_ev;

    for (uint32_t i = 0; i < KEY_MAX; ++i) {
        if (is_button_down(state[i], _G.state[i])) {
            keyboard_ev.keycode = i;
            eventstream::push<ct_event_header>(stream, EVENT_KEYBOARD_DOWN,
                                               keyboard_ev);

        } else if (is_button_up(state[i], _G.state[i])) {
            keyboard_ev.keycode = i;
            eventstream::push<ct_event_header>(stream, EVENT_KEYBOARD_UP,
                                               keyboard_ev);

        }

        _G.state[i] = state[i];
    }
}


void sdl_keyboard_process(SDL_Event *e,
                          EventStream &stream) {
    switch (e->type) {
        case SDL_TEXTINPUT: {
            ct_keyboard_text_event ev;
            memcpy(ev.text, e->text.text, sizeof(ev.text));

            eventstream::push<ct_event_header>(stream, EVENT_KEYBOARD_TEXT, ev);
        }
            break;
    }
}

