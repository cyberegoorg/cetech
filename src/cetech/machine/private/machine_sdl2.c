//==============================================================================
// Includes
//==============================================================================

#include "celib/memory.h"
#include "celib/module.h"
#include "celib/api_system.h"
#include "celib/log.h"
#include "celib/cdb.h"
#include <celib/hashlib.h>

#include <celib/os.h>
#include <celib/macros.h>

#include "cetech/machine/machine.h"

#include <include/SDL2/SDL.h>
#include <cetech/renderer/renderer.h>
#include <cetech/controlers/mouse.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/controlers/gamepad.h>
#include <cetech/kernel/kernel.h>
#include <cetech/controlers/controlers.h>
#include <celib/array.inl>

extern int sdl_window_init(struct ce_api_a0 *api);

extern void sdl_window_shutdown();

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "machine"
#define GAMEPAD_MAX 8

//==============================================================================
// Globals
//==============================================================================

#define is_button_down(now, last) ((now) && !(last))
#define is_button_up(now, last)   (!(now) && (last))


static struct MachineGlobals {
    struct {
        uint8_t state[MOUSE_BTN_MAX];
        float position[2];
    } mouse;

    struct {
        SDL_GameController *controller[GAMEPAD_MAX];
        SDL_Haptic *haptic[GAMEPAD_MAX];

        float position[GAMEPAD_MAX][GAMEPAD_AXIX_MAX][2];
        int state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
    } controlers;

    struct {
        uint8_t state[KEY_MAX];
    } keyboard;

    uint64_t *events;
} _G = {};


//==============================================================================
// Interface
//==============================================================================


static void _add_events(uint64_t event) {
    ce_array_push(_G.events, event, ce_memory_a0->system);
}

void sdl_mouse_process() {
    int pos[2] = {};

    uint32_t state = SDL_GetMouseState(&pos[0], &pos[1]);

    uint8_t curent_state[MOUSE_BTN_MAX] = {};

    curent_state[MOUSE_BTN_LEFT] = (uint8_t) (state & SDL_BUTTON_LMASK);
    curent_state[MOUSE_BTN_RIGHT] = (uint8_t) (state & SDL_BUTTON_RMASK);
    curent_state[MOUSE_BTN_MIDLE] = (uint8_t) (state & SDL_BUTTON_MMASK);

    struct ct_renderer_a0 *renderer_a0 = (struct ct_renderer_a0 *) ce_api_a0->first(
            ce_id_a0->id64("ct_renderer_a0")).api;

    uint32_t window_size[2] = {};
    renderer_a0->get_size(&window_size[0], &window_size[1]);

    _G.mouse.position[0] = pos[0];
    _G.mouse.position[1] = window_size[1] - pos[1];


    uint64_t event;
    event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                     EVENT_MOUSE_MOVE);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), event);
    ce_cdb_a0->set_float(w, CONTROLER_POSITION_X, _G.mouse.position[0]);
    ce_cdb_a0->set_float(w, CONTROLER_POSITION_Y, _G.mouse.position[1]);
    ce_cdb_a0->write_commit(w);

    _add_events(event);

    for (uint32_t i = 0; i < MOUSE_BTN_MAX; ++i) {
        if (is_button_down(curent_state[i], _G.mouse.state[i])) {
            event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                             EVENT_MOUSE_DOWN);

            w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), event);
            ce_cdb_a0->set_uint64(w, CONTROLER_BUTTON, i);
            ce_cdb_a0->write_commit(w);

            _add_events(event);

        } else if (is_button_up(curent_state[i], _G.mouse.state[i])) {
            event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                             EVENT_MOUSE_UP);

            w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), event);
            ce_cdb_a0->set_uint64(w, CONTROLER_BUTTON, i);
            ce_cdb_a0->write_commit(w);

            _add_events(event);
        }

        _G.mouse.state[i] = curent_state[i];
    }
}

void sdl_keyboard_process() {
    const uint8_t *state = SDL_GetKeyboardState(NULL);


    uint64_t event;
    for (uint32_t i = 0; i < KEY_MAX; ++i) {
        if (is_button_down(state[i], _G.keyboard.state[i])) {
            event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                             EVENT_KEYBOARD_DOWN);

            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), event);
            ce_cdb_a0->set_uint64(w, CONTROLER_KEYCODE, i);
            ce_cdb_a0->write_commit(w);

            _add_events(event);


        } else if (is_button_up(state[i], _G.keyboard.state[i])) {
            event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                             EVENT_KEYBOARD_UP);

            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), event);
            ce_cdb_a0->set_uint64(w, CONTROLER_KEYCODE, i);
            ce_cdb_a0->write_commit(w);

            _add_events(event);

        }

        _G.keyboard.state[i] = state[i];
    }
}

int _new_controler() {
    for (int i = 0; i < GAMEPAD_MAX; ++i) {
        if (_G.controlers.controller[i] != NULL) {
            continue;
        }

        return i;
    }

    return -1;
}

void _remove_controler(int idx) {
    ce_log_a0->info("controlers.gamepad", "Remove gamepad %d.", idx);

    SDL_HapticClose(_G.controlers.haptic[idx]);
    SDL_GameControllerClose(_G.controlers.controller[idx]);

    _G.controlers.controller[idx] = NULL;
    _G.controlers.haptic[idx] = NULL;
}

int _create_controler(int i) {
    SDL_GameController *controller = SDL_GameControllerOpen(i);
    SDL_Joystick *joy = SDL_GameControllerGetJoystick(controller);

    int idx = _new_controler();

    _G.controlers.controller[idx] = controller;

    memset(_G.controlers.state[idx], 0, sizeof(int) * GAMEPAD_BTN_MAX);

    ce_log_a0->info("controlers.gamepad", "Add gamepad %d.", i);

    if (SDL_JoystickIsHaptic(joy) == 1) {
        SDL_Haptic *haptic = SDL_HapticOpenFromJoystick(joy);
        SDL_HapticRumbleInit(haptic);
        _G.controlers.haptic[idx] = haptic;

        ce_log_a0->info("controlers.gamepad", "Gamepad %d has haptic support.",
                        i);
    } else {
        _G.controlers.haptic[idx] = NULL;
    }

    return idx;
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

void sdl_gamepad_process() {
    int curent_state[GAMEPAD_MAX][GAMEPAD_AXIX_MAX] = {};
    float curent_pos[GAMEPAD_MAX][GAMEPAD_BTN_MAX][2] = {};

    for (int i = 0; i < GAMEPAD_MAX; ++i) {
        if (_G.controlers.controller[i] == NULL) {
            continue;
        }

        for (int j = 0; j < GAMEPAD_BTN_MAX; ++j) {
            curent_state[i][j] = SDL_GameControllerGetButton(
                    _G.controlers.controller[i],
                    _btn_to_sdl[j]);
        }

        for (int j = 0; j < GAMEPAD_AXIX_MAX; ++j) {
            int16_t x = SDL_GameControllerGetAxis(_G.controlers.controller[i],
                                                  _axis_to_sdl[j][0]);
            int16_t y = SDL_GameControllerGetAxis(_G.controlers.controller[i],
                                                  _axis_to_sdl[j][1]);

            float coef = 1.0f / INT16_MAX;
            float x_norm = x * coef;
            float y_norm = y * coef;

            curent_pos[i][j][0] = x_norm;
            curent_pos[i][j][1] = -y_norm;
        }
    }

    for (int i = 0; i < GAMEPAD_MAX; ++i) {
        if (_G.controlers.controller[i] == NULL) {
            continue;
        }

        for (int j = 0; j < GAMEPAD_BTN_MAX; ++j) {
            if (is_button_down(curent_state[i][j], _G.controlers.state[i][j])) {
                uint64_t event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                          EVENT_GAMEPAD_DOWN);

                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                         event);
                ce_cdb_a0->set_uint64(w, CONTROLER_ID, i);
                ce_cdb_a0->set_uint64(w, CONTROLER_BUTTON, j);
                ce_cdb_a0->write_commit(w);

                _add_events(event);


            } else if (is_button_up(curent_state[i][j],
                                    _G.controlers.state[i][j])) {
                uint64_t event = ce_cdb_a0->create_object(
                        ce_cdb_a0->db(),
                        EVENT_GAMEPAD_UP);

                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                         event);
                ce_cdb_a0->set_uint64(w, CONTROLER_ID, i);
                ce_cdb_a0->set_uint64(w, CONTROLER_BUTTON, j);
                ce_cdb_a0->write_commit(w);

                _add_events(event);

            }

            _G.controlers.state[i][j] = curent_state[i][j];
        }

        for (int j = 0; j < GAMEPAD_AXIX_MAX; ++j) {
            float pos[2] = {curent_pos[i][j][0], curent_pos[i][j][1]};


            if ((pos[0] != _G.controlers.position[i][j][0]) ||
                (pos[1] != _G.controlers.position[i][j][1])) {

                _G.controlers.position[i][j][0] = pos[0];
                _G.controlers.position[i][j][1] = pos[1];

                uint64_t event = ce_cdb_a0->create_object(
                        ce_cdb_a0->db(),
                        EVENT_GAMEPAD_MOVE);

                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                         event);
                ce_cdb_a0->set_uint64(w, CONTROLER_ID, i);
                ce_cdb_a0->set_uint64(w, CONTROLER_AXIS, j);

                ce_cdb_a0->set_float(w,
                                     CONTROLER_POSITION_X, pos[0]);
                ce_cdb_a0->set_float(w,
                                     CONTROLER_POSITION_Y, pos[1]);
                ce_cdb_a0->write_commit(w);

                _add_events(event);

            }
        }
    }
}

int sdl_gamepad_is_active(int idx) {
    return _G.controlers.controller[idx] != NULL;
}

void sdl_gamepad_play_rumble(int gamepad,
                             float strength,
                             uint32_t length) {

    SDL_Haptic *h = _G.controlers.haptic[gamepad];

    SDL_HapticRumblePlay(h, strength, length);
}

static void _update(float dt) {
    const uint32_t event_n = ce_array_size(_G.events);
    for (int j = 0; j < event_n; ++j) {
        ce_cdb_a0->destroy_object(ce_cdb_a0->db(), _G.events[j]);
    }
    ce_array_clean(_G.events);

    SDL_Event e = {};

    while (SDL_PollEvent(&e) > 0) {
        switch (e.type) {
            case SDL_QUIT:
                ct_kernel_a0->quit();
                break;

            case SDL_WINDOWEVENT: {
                switch (e.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        uint64_t event = ce_cdb_a0->create_object(
                                ce_cdb_a0->db(),
                                EVENT_WINDOW_RESIZED);

                        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(
                                ce_cdb_a0->db(), event);

                        ce_cdb_a0->set_uint64(w, CT_MACHINE_WINDOW_ID,
                                              e.window.windowID);
                        ce_cdb_a0->set_uint64(w, CT_MACHINE_WINDOW_WIDTH,
                                              e.window.data1);
                        ce_cdb_a0->set_uint64(w, CT_MACHINE_WINDOW_HEIGHT,
                                              e.window.data2);
                        ce_cdb_a0->write_commit(w);

                        _add_events(event);

                    }
                        break;
                }
            }
                break;

            case SDL_MOUSEWHEEL: {
                uint64_t event = ce_cdb_a0->create_object(
                        ce_cdb_a0->db(),
                        EVENT_MOUSE_WHEEL);

                float pos[3] = {e.wheel.x, e.wheel.y};

                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                         event);
                ce_cdb_a0->set_float(w, CONTROLER_POSITION_X, pos[0]);
                ce_cdb_a0->set_float(w, CONTROLER_POSITION_Y, pos[1]);
                ce_cdb_a0->write_commit(w);

                _add_events(event);

            }
                break;


            case SDL_TEXTINPUT: {
                uint64_t event = ce_cdb_a0->create_object(
                        ce_cdb_a0->db(),
                        EVENT_KEYBOARD_TEXT);


                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                         event);
                ce_cdb_a0->set_str(w, CONTROLER_TEXT, e.text.text);
                ce_cdb_a0->write_commit(w);

                _add_events(event);

            }
                break;

            case SDL_CONTROLLERDEVICEADDED: {
                int idx = _create_controler(e.cdevice.which);

                uint64_t event = ce_cdb_a0->create_object(
                        ce_cdb_a0->db(),
                        EVENT_GAMEPAD_CONNECT);

                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                         event);
                ce_cdb_a0->set_uint64(w, CONTROLER_ID, idx);
                ce_cdb_a0->write_commit(w);

                _add_events(event);

            }
                break;

            case SDL_CONTROLLERDEVICEREMOVED: {
                for (int i = 0; i < GAMEPAD_MAX; ++i) {
                    SDL_GameController *controller;
                    controller = SDL_GameControllerFromInstanceID(
                            e.cdevice.which);

                    if (_G.controlers.controller[i] != controller) {
                        continue;
                    }

                    _remove_controler(i);

                    uint64_t event = ce_cdb_a0->create_object(
                            ce_cdb_a0->db(),
                            EVENT_GAMEPAD_DISCONNECT);

                    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                             event);
                    ce_cdb_a0->set_uint64(w, CONTROLER_ID, i);
                    ce_cdb_a0->write_commit(w);

                    _add_events(event);

                    break;
                }
            }
                break;


            default:
                break;
        }
    }

    sdl_gamepad_process();
    sdl_mouse_process();
    sdl_keyboard_process();
}


static const uint64_t *events(uint64_t *n) {
    *n = ce_array_size(_G.events);
    return _G.events;
}

static struct ct_machine_a0 a0 = {
        .gamepad_is_active = sdl_gamepad_is_active,
        .gamepad_play_rumble = sdl_gamepad_play_rumble,
        .events = events,
};

struct ct_machine_a0 *ct_machine_a0 = &a0;

static uint64_t task_name() {
    return CT_MACHINE_TASK;
}

static uint64_t *update_before(uint64_t *n) {
    static uint64_t a[] = {
            CT_INPUT_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}

static struct ct_kernel_task_i0 machine_task = {
        .name = task_name,
        .update = _update,
        .update_before = update_before,
};

static void init(struct ce_api_a0 *api) {
    api->register_api(CT_MACHINE_API, &a0);
    api->register_api(KERNEL_TASK_INTERFACE, &machine_task);

    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);

    if (SDL_Init(SDL_INIT_VIDEO |
                 SDL_INIT_GAMECONTROLLER |
                 SDL_INIT_HAPTIC |
                 SDL_INIT_JOYSTICK) != 0) {
        //if (SDL_Init(0) != 0) {
        ce_log_a0->error(LOG_WHERE, "Could not init sdl - %s",
                         SDL_GetError());
        return; // TODO: dksandasdnask FUCK init without return ptype?????
    }

    sdl_window_init(api);
}

static void shutdown() {
    sdl_window_shutdown();

    SDL_Quit();
}

CE_MODULE_DEF(
        machine,
        {
            CE_INIT_API(api, ce_log_a0);

            ce_api_a0 = api;
        },
        {
            CE_UNUSED(reload);
            init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            shutdown();

        }
)
