//==============================================================================
// Incldues
//==============================================================================

#include <include/SDL2/SDL.h>
#include <cetech/machine/types.h>


//==============================================================================
// Defines
//==============================================================================

#define is_button_down(now, last) ((now) && !(last))
#define is_button_up(now, last)   (!(now) && (last))


//==============================================================================
// Globals
//==============================================================================

static struct G {
    u8 last_state[MOUSE_BTN_MAX];
    int last_position[2];
} _G = {0};


//==============================================================================
// Interface
//==============================================================================

int sdl_mouse_init(){
    _G = (struct G) {0};

    return 1;
}

void sdl_mouse_shutdown() {
    _G = (struct G) {0};
}

void sdl_mouse_process(struct eventstream *stream) {
    int pos[2] = {0};

    u32 state = SDL_GetMouseState(&pos[0], &pos[1]);

    u8 curent_state[MOUSE_BTN_MAX] = {0};

    curent_state[MOUSE_BTN_LEFT] = (u8) (state & SDL_BUTTON_LMASK);
    curent_state[MOUSE_BTN_RIGHT] = (u8) (state & SDL_BUTTON_RMASK);
    curent_state[MOUSE_BTN_MIDLE] = (u8) (state & SDL_BUTTON_MMASK);

    if ((pos[0] != _G.last_position[0]) || (pos[1] != _G.last_position[1])) {
        _G.last_position[0] = pos[0];
        _G.last_position[1] = pos[1];

        struct mouse_move_event event;
        event.pos[0] = pos[0];
        event.pos[1] = pos[1];

        event_stream_push(stream, EVENT_MOUSE_MOVE, event);
    }

    for (u32 i = 0; i < MOUSE_BTN_MAX; ++i) {
        struct mouse_event event;
        event.button = i;

        if (is_button_down(curent_state[i], _G.last_state[i]))
            event_stream_push(stream, EVENT_MOUSE_DOWN, event);

        else if (is_button_up(curent_state[i], _G.last_state[i]))
            event_stream_push(stream, EVENT_MOUSE_UP, event);

        _G.last_state[i] = curent_state[i];
    }
}