//==============================================================================
// Incldues
//==============================================================================

#include <include/SDL2/SDL.h>

#include <cetech/core/module.h>
#include <cetech/core/container/eventstream.inl>
#include <cetech/core/os/window.h>

#include <cetech/core/application.h>
#include <cetech/core/resource/resource.h>
#include <cetech/core/machine.h>
#include <cetech/core/api.h>


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

int sdl_mouse_init(struct api_v0 *api) {
    _G = (struct G) {0};

    GET_API(api, app_api_v0);
    GET_API(api, window_api_v0);

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
        window_t main_window = app_api_v0.main_window();
        uint32_t window_size[2] = {0};
        window_api_v0.get_size(main_window, &window_size[0],
                               &window_size[1]);

        _G.position[0] = pos[0];
        _G.position[1] = window_size[1] - pos[1];

        struct mouse_move_event event;
        event.pos.x = pos[0];
        event.pos.y = window_size[1] - pos[1];

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
