//==============================================================================
// Incldues
//==============================================================================

#include <include/SDL2/SDL.h>
#include <cetech/application.h>
#include <cetech/module.h>
#include <cetech/machine.h>


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
    int position[2];
} _G = {0};


IMPORT_API(app_api_v0);
IMPORT_API(window_api_v0);

//==============================================================================
// Interface
//==============================================================================

int sdl_mouse_init(get_api_fce_t get_engine_api) {
    _G = (struct G) {0};

    INIT_API(get_engine_api, app_api_v0, APPLICATION_API_ID);
    INIT_API(get_engine_api, window_api_v0, WINDOW_API_ID);

    return 1;
}

void sdl_mouse_shutdown() {
    _G = (struct G) {0};
}

void sdl_mouse_process(struct eventstream *stream) {
    int pos[2] = {0};

    uint32_t state = SDL_GetMouseState(&pos[0], &pos[1]);

    uint8_t curent_state[MOUSE_BTN_MAX] = {0};

    curent_state[MOUSE_BTN_LEFT] = (uint8_t) (state & SDL_BUTTON_LMASK);
    curent_state[MOUSE_BTN_RIGHT] = (uint8_t) (state & SDL_BUTTON_RMASK);
    curent_state[MOUSE_BTN_MIDLE] = (uint8_t) (state & SDL_BUTTON_MMASK);

    if ((pos[0] != _G.position[0]) || (pos[1] != _G.position[1])) {
        cel_window_t main_window = app_api_v0.main_window();
        uint32_t cel_window_size[2] = {0};
        window_api_v0.get_size(main_window, &cel_window_size[0],
                                          &cel_window_size[1]);

        _G.position[0] = pos[0];
        _G.position[1] = cel_window_size[1] - pos[1];

        struct mouse_move_event event;
        event.pos.x = pos[0];
        event.pos.y = cel_window_size[1] - pos[1];

        event_stream_push(stream, EVENT_MOUSE_MOVE, event);
    }

    for (uint32_t i = 0; i < MOUSE_BTN_MAX; ++i) {
        struct mouse_event event;
        event.button = i;

        if (is_button_down(curent_state[i], _G.state[i]))
            event_stream_push(stream, EVENT_MOUSE_DOWN, event);

        else if (is_button_up(curent_state[i], _G.state[i]))
            event_stream_push(stream, EVENT_MOUSE_UP, event);

        _G.state[i] = curent_state[i];
    }
}
