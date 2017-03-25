//==============================================================================
// Includes
//==============================================================================

#include <include/SDL2/SDL.h>
#include <engine/module/api.h>
#include "engine/machine/api.h"
//==============================================================================
// Defines
//==============================================================================

#define is_button_down(now, last) ((now) && !(last))
#define is_button_up(now, last)   (!(now) && (last))


//==============================================================================
// Globals
//==============================================================================

static struct G {
    u8 state[KEY_MAX];
} _G = {0};


//==============================================================================
// Interface
//==============================================================================

int sdl_keyboard_init(get_api_fce_t get_engine_api) {
    _G = (struct G) {0};

    return 1;
}

void sdl_keyboard_shutdown() {
    _G = (struct G) {0};
}

void sdl_keyboard_process(struct eventstream *stream) {
    const u8 *state = SDL_GetKeyboardState(NULL);
    struct keyboard_event keyboard_ev;

    for (u32 i = 0; i < KEY_MAX; ++i) {
        if (is_button_down(state[i], _G.state[i])) {
            keyboard_ev.keycode = i;
            event_stream_push(stream, EVENT_KEYBOARD_DOWN, keyboard_ev);

        } else if (is_button_up(state[i], _G.state[i])) {
            keyboard_ev.keycode = i;
            event_stream_push(stream, EVENT_KEYBOARD_UP, keyboard_ev);

        }

        _G.state[i] = state[i];
    }
}
