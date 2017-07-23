//==============================================================================
// Incldues
//==============================================================================

#include <include/SDL2/SDL.h>

#include <cetech/kernel/os.h>
#include <cetech/celib/eventstream.inl>

#include <cetech/kernel/api_system.h>
#include <cetech/kernel/log.h>

//==============================================================================
// Defines
//==============================================================================

#define is_button_down(now, last) ((now) && !(last))
#define is_button_up(now, last)   (!(now) && (last))

using namespace celib;

//==============================================================================
// Globals
//==============================================================================

static struct G {
    //uint8_t state[MOUSE_BTN_MAX];
    //int position[2];

    SDL_GameController *controller[GAMEPAD_MAX];
    SDL_Haptic *haptic[GAMEPAD_MAX];

    float position[GAMEPAD_MAX][GAMEPAD_AXIX_MAX][2];
    int state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
} _G = {0};


CETECH_DECL_API(ct_log_a0)


int _new_controler() {
    for (int i = 0; i < GAMEPAD_MAX; ++i) {
        if (_G.controller[i] != NULL) {
            continue;
        }

        return i;
    }

    return -1;
}

void _remove_controler(int idx) {
    _G.controller[idx] = NULL;
    _G.haptic[idx] = NULL;
}

int _create_controler(int i) {
    SDL_GameController *controller = SDL_GameControllerOpen(i);
    SDL_Joystick *joy = SDL_GameControllerGetJoystick(controller);

    int idx = _new_controler();

    _G.controller[idx] = controller;

    memset(_G.state[idx], 0, sizeof(int) * GAMEPAD_BTN_MAX);

    if (SDL_JoystickIsHaptic(joy) == 1) {
        SDL_Haptic *haptic = SDL_HapticOpenFromJoystick(joy);
        SDL_HapticRumbleInit(haptic);
        _G.haptic[idx] = haptic;

        ct_log_a0.info("input.gamepad", "Gamepad %d has haptic support",
                       i);
    } else {
        _G.haptic[idx] = NULL;
    }

    return idx;
}

//==============================================================================
// Interface
//==============================================================================

int sdl_gamepad_init(ct_api_a0 *api) {
    CETECH_GET_API(api, ct_log_a0);

    _G = (struct G) {0};

    int num_joy = SDL_NumJoysticks();
    for (int i = 0; i < num_joy; ++i) {
        if (SDL_IsGameController(i) == SDL_FALSE) {
            continue;
        }

        //_create_controler(i);
    }

    return 1;
}

void sdl_gamepad_shutdown() {
    _G = (struct G) {0};
}

static SDL_GameControllerButton _btn_to_sdl[GAMEPAD_BTN_MAX] = {
        [GAMEPAD_BTN_INVALID] = SDL_CONTROLLER_BUTTON_INVALID,
        [GAMEPAD_BTN_A] =               SDL_CONTROLLER_BUTTON_A,
        [GAMEPAD_BTN_B] =               SDL_CONTROLLER_BUTTON_B,
        [GAMEPAD_BTN_X] =               SDL_CONTROLLER_BUTTON_X,
        [GAMEPAD_BTN_Y] =               SDL_CONTROLLER_BUTTON_Y,
        [GAMEPAD_BTN_BACK] =            SDL_CONTROLLER_BUTTON_BACK,
        [GAMEPAD_BTN_GUIDE] =           SDL_CONTROLLER_BUTTON_GUIDE,
        [GAMEPAD_BTN_START] =           SDL_CONTROLLER_BUTTON_START,
        [GAMEPAD_BTN_LEFTSTICK] =       SDL_CONTROLLER_BUTTON_LEFTSTICK,
        [GAMEPAD_BTN_RIGHTSTICK] =      SDL_CONTROLLER_BUTTON_RIGHTSTICK,
        [GAMEPAD_BTN_LEFTSHOULDER] =    SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
        [GAMEPAD_BTN_RIGHTSHOULDER] =   SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        [GAMEPAD_BTN_DPADUP] =          SDL_CONTROLLER_BUTTON_DPAD_UP,
        [GAMEPAD_BTN_DPADDOWN] =        SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        [GAMEPAD_BTN_DPADLEFT] =        SDL_CONTROLLER_BUTTON_DPAD_LEFT,
        [GAMEPAD_BTN_DPADRIGHT] =       SDL_CONTROLLER_BUTTON_DPAD_RIGHT
};

static SDL_GameControllerAxis _axis_to_sdl[GAMEPAD_AXIX_MAX][2] = {
        [GAMEPAD_AXIS_INVALID] = {SDL_CONTROLLER_AXIS_INVALID,
                                  SDL_CONTROLLER_AXIS_INVALID},
        [GAMEPAD_AXIS_LEFT] = {SDL_CONTROLLER_AXIS_LEFTX,
                               SDL_CONTROLLER_AXIS_LEFTY},
        [GAMEPAD_AXIS_RIGHT] = {SDL_CONTROLLER_AXIS_RIGHTX,
                                SDL_CONTROLLER_AXIS_RIGHTY},
        [GAMEPAD_AXIS_TRIGER] = {SDL_CONTROLLER_AXIS_TRIGGERLEFT,
                                 SDL_CONTROLLER_AXIS_TRIGGERRIGHT},
};

void sdl_gamepad_process(EventStream &stream) {
    int curent_state[GAMEPAD_MAX][GAMEPAD_AXIX_MAX] = {0};
    float curent_pos[GAMEPAD_MAX][GAMEPAD_BTN_MAX][2] = {0};

    for (int i = 0; i < GAMEPAD_MAX; ++i) {
        if (_G.controller[i] == NULL) {
            continue;
        }

        for (int j = 0; j < GAMEPAD_BTN_MAX; ++j) {
            curent_state[i][j] = SDL_GameControllerGetButton(_G.controller[i],
                                                             _btn_to_sdl[j]);
        }

        for (int j = 0; j < GAMEPAD_AXIX_MAX; ++j) {
            int16_t x = SDL_GameControllerGetAxis(_G.controller[i],
                                                  _axis_to_sdl[j][0]);
            int16_t y = SDL_GameControllerGetAxis(_G.controller[i],
                                                  _axis_to_sdl[j][1]);

            float coef = 1.0f / INT16_MAX;
            float x_norm = x * coef;
            float y_norm = y * coef;

            curent_pos[i][j][0] = x_norm;
            curent_pos[i][j][1] = -y_norm;
        }
    }

    for (int i = 0; i < GAMEPAD_MAX; ++i) {
        if (_G.controller[i] == NULL) {
            continue;
        }

        for (int j = 0; j < GAMEPAD_BTN_MAX; ++j) {
            ct_gamepad_btn_event event;
            event.gamepad_id = i;
            event.button = j;

            if (is_button_down(curent_state[i][j], _G.state[i][j]))
                eventstream::push(stream, EVENT_GAMEPAD_DOWN, event);

            else if (is_button_up(curent_state[i][j], _G.state[i][j]))
                eventstream::push(stream, EVENT_GAMEPAD_UP, event);

            _G.state[i][j] = curent_state[i][j];
        }

        for (int j = 0; j < GAMEPAD_AXIX_MAX; ++j) {
            ct_gamepad_move_event event;
            event.gamepad_id = i;
            event.axis = j;

            float pos[2] = {curent_pos[i][j][0], curent_pos[i][j][1]};


            if ((pos[0] != _G.position[i][j][0]) ||
                (pos[1] != _G.position[i][j][1])) {

                _G.position[i][j][0] = pos[0];
                _G.position[i][j][1] = pos[1];

                event.position[0] = pos[0];
                event.position[1] = pos[1];

                eventstream::push(stream, EVENT_GAMEPAD_MOVE, event);
            }
        }
    }
}

void sdl_gamepad_process_event(SDL_Event *event,
                               EventStream &stream) {
    switch (event->type) {
        case SDL_CONTROLLERDEVICEADDED: {
            int idx = _create_controler(event->cdevice.which);
            ct_gamepad_device_event ev;
            ev.gamepad_id = idx;

            eventstream::push(stream, EVENT_GAMEPAD_CONNECT, ev);
        }
            break;

        case SDL_CONTROLLERDEVICEREMOVED: {
            ct_gamepad_device_event ev;

            for (int i = 0; i < GAMEPAD_MAX; ++i) {
                SDL_GameController *controller = SDL_GameControllerFromInstanceID(
                        event->cdevice.which);
                if (_G.controller[i] != controller) {
                    continue;
                }

                _remove_controler(i);

                ev.gamepad_id = i;
                eventstream::push(stream, EVENT_GAMEPAD_DISCONNECT, ev);

                break;
            }
        }
            break;


        default:
            break;
    }
}

int sdl_gamepad_is_active(int idx) {
    return _G.controller[idx] != NULL;
}

void sdl_gamepad_play_rumble(int gamepad,
                             float strength,
                             uint32_t length) {

    SDL_Haptic *h = _G.haptic[gamepad];

    SDL_HapticRumblePlay(h, strength, length);
}