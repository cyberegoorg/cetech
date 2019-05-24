//==============================================================================
// Includes
//==============================================================================

#include "celib/memory/memory.h"
#include "celib/module.h"
#include "celib/api.h"
#include "celib/log.h"
#include "celib/cdb.h"
#include <celib/id.h>
#include <celib/memory/allocator.h>


#include <celib/macros.h>

#include "cetech/machine/machine.h"

#include <include/SDL2/SDL.h>

#include <cetech/renderer/renderer.h>
#include <cetech/controlers/mouse.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/controlers/gamepad.h>
#include <cetech/kernel/kernel.h>
#include <cetech/controlers/controlers.h>
#include <celib/containers/array.h>
#include <celib/os/window.h>
#include <stdatomic.h>
#include <celib/containers/mpmc.h>
#include <celib/math/math.h>

extern int sdl_window_init(struct ce_api_a0 *api);

extern void sdl_window_shutdown();

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "machine"
#define GAMEPAD_MAX 8
#define MAX_EVENTS_LISTENER 32

//==============================================================================
// Globals
//==============================================================================

#define is_button_down(now, last) ((now) && !(last))
#define is_button_up(now, last)   (!(now) && (last))

typedef struct listener_pack_t {
    ce_mpmc_queue_t0 *queues;
    atomic_uint_fast16_t n;
} listener_pack_t;

#define _G MachineGlobals

static struct _G {
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

    listener_pack_t obj_listeners;
    ce_alloc_t0 *allocator;
} _G = {};


static void _init_listener_pack(listener_pack_t *pack) {
    pack->queues = CE_REALLOC(ce_memory_a0->virt_system, ce_mpmc_queue_t0,
                              NULL, sizeof(ce_mpmc_queue_t0) * MAX_EVENTS_LISTENER, 0);
}

static ce_mpmc_queue_t0 *_new_listener(listener_pack_t *pack,
                                       uint32_t queue_size,
                                       size_t item_size) {
    uint32_t idx = atomic_fetch_add(&pack->n, 1);
    ce_mpmc_queue_t0 *q = &pack->queues[idx];
    ce_mpmc_init(q, queue_size, item_size, _G.allocator);
    return q;
}

static void _push_event(listener_pack_t *pack,
                        void *event) {
    uint32_t n = pack->n;
    for (int i = 0; i < n; ++i) {
        ce_mpmc_queue_t0 *q = &pack->queues[i];
        ce_mpmc_enqueue(q, event);
    }
}

//==============================================================================
// Interface
//==============================================================================


ct_machine_ev_queue_o0 *new_ev_listener() {
    return (ct_machine_ev_queue_o0 *) _new_listener(&_G.obj_listeners,
                                                    4096, sizeof(ct_machine_ev_t0));
}

bool pop_ev(ct_machine_ev_queue_o0 *q,
            ct_machine_ev_t0 *ev) {
    ce_mpmc_queue_t0 *qq = (ce_mpmc_queue_t0 *) q;
    return ce_mpmc_dequeue(qq, ev);
}


static void _add_events(ct_machine_ev_t0 event) {
    _push_event(&_G.obj_listeners, &event);
}

void sdl_mouse_process() {
    int pos[2] = {};

    uint32_t state = SDL_GetMouseState(&pos[0], &pos[1]);

    uint8_t curent_state[MOUSE_BTN_MAX] = {};

    curent_state[MOUSE_BTN_LEFT] = (uint8_t) (state & SDL_BUTTON_LMASK);
    curent_state[MOUSE_BTN_RIGHT] = (uint8_t) (state & SDL_BUTTON_RMASK);
    curent_state[MOUSE_BTN_MIDLE] = (uint8_t) (state & SDL_BUTTON_MMASK);

    uint32_t window_size[2] = {};
    ct_renderer_a0->get_size(&window_size[0], &window_size[1]);

    bool mose_move = false;
    if (!ce_fequal(_G.mouse.position[0], pos[0], 0.000001)) {
        _G.mouse.position[0] = pos[0];
        mose_move = true;
    }

    if (!ce_fequal(_G.mouse.position[1], window_size[1] - pos[1], 0.000001)) {
        _G.mouse.position[1] = window_size[1] - pos[1];
        mose_move = true;
    }

    if (mose_move) {
        _add_events((ct_machine_ev_t0) {
                .ev_type = EVENT_MOUSE_MOVE,
                .mouse = {
                        .pos = {.x = _G.mouse.position[0], .y = _G.mouse.position[1]}
                }
        });
    }

    for (uint32_t i = 0; i < MOUSE_BTN_MAX; ++i) {
        if (is_button_down(curent_state[i], _G.mouse.state[i])) {
            _add_events((ct_machine_ev_t0) {
                    .ev_type = EVENT_MOUSE_DOWN,
                    .mouse = {.btn = i,},
            });


        } else if (is_button_up(curent_state[i], _G.mouse.state[i])) {
            _add_events((ct_machine_ev_t0) {
                    .ev_type = EVENT_MOUSE_UP,
                    .mouse = {.btn = i,},
            });
        }

        _G.mouse.state[i] = curent_state[i];
    }
}

void sdl_keyboard_process() {
    const uint8_t *state = SDL_GetKeyboardState(NULL);

    for (uint32_t i = 0; i < KEY_MAX; ++i) {
        if (is_button_down(state[i], _G.keyboard.state[i])) {
            _add_events((ct_machine_ev_t0) {
                    .ev_type = EVENT_KEYBOARD_DOWN,
                    .key = {.keycode = i}
            });

        } else if (is_button_up(state[i], _G.keyboard.state[i])) {
            _add_events((ct_machine_ev_t0) {
                    .ev_type = EVENT_KEYBOARD_UP,
                    .key = {.keycode = i}
            });

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

        ce_log_a0->info("controlers.gamepad", "Gamepad %d has haptic support.", i);
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
                _add_events((ct_machine_ev_t0) {
                        .ev_type = EVENT_GAMEPAD_DOWN,
                        .gamepad = {
                                .gamepad_id = i,
                                .btn = j,
                        }
                });

            } else if (is_button_up(curent_state[i][j],
                                    _G.controlers.state[i][j])) {
                _add_events((ct_machine_ev_t0) {
                        .ev_type = EVENT_GAMEPAD_UP,
                        .gamepad = {
                                .gamepad_id = i,
                                .btn = j,
                        }
                });
            }

            _G.controlers.state[i][j] = curent_state[i][j];
        }

        for (int j = 0; j < GAMEPAD_AXIX_MAX; ++j) {
            float pos[2] = {curent_pos[i][j][0], curent_pos[i][j][1]};


            if ((pos[0] != _G.controlers.position[i][j][0]) ||
                (pos[1] != _G.controlers.position[i][j][1])) {

                _G.controlers.position[i][j][0] = pos[0];
                _G.controlers.position[i][j][1] = pos[1];

                _add_events((ct_machine_ev_t0) {
                        .ev_type = EVENT_GAMEPAD_MOVE,
                        .gamepad = {
                                .gamepad_id = i,
                                .axis_id = j,
                                .pos = {.x = pos[0], .y = pos[1]}
                        }
                });

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
    SDL_Event e = {};

    while (SDL_PollEvent(&e) > 0) {
        switch (e.type) {
            case SDL_QUIT:
                ct_kernel_a0->quit();
                break;

            case SDL_WINDOWEVENT: {
                switch (e.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        _add_events((ct_machine_ev_t0) {
                                .ev_type = EVENT_WINDOW_RESIZED,
                                .window_resize = {
                                        .window_id = e.window.windowID,
                                        .width = e.window.data1,
                                        .height= e.window.data2,
                                }
                        });

                    }
                        break;
                }
            }
                break;

            case SDL_MOUSEWHEEL: {
                _add_events((ct_machine_ev_t0) {
                        .ev_type = EVENT_MOUSE_WHEEL,
                        .mouse = {
                                .pos = {.x = e.wheel.x, .y = e.wheel.y}
                        }
                });

            }
                break;


            case SDL_TEXTINPUT: {
                ct_machine_ev_t0 ev = {.ev_type=EVENT_KEYBOARD_TEXT};
                memcpy(ev.key.text, e.text.text, sizeof(char[32]));
                _add_events(ev);
            }
                break;

            case SDL_CONTROLLERDEVICEADDED: {
                int idx = _create_controler(e.cdevice.which);

                _add_events((ct_machine_ev_t0) {
                        .ev_type = EVENT_GAMEPAD_CONNECT,
                        .gamepad = {.gamepad_id = idx}
                });

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

                    _add_events((ct_machine_ev_t0) {
                            .ev_type = EVENT_GAMEPAD_DISCONNECT,
                            .gamepad = {.gamepad_id = i}
                    });

                    _remove_controler(i);
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

static struct ct_machine_a0 a0 = {
        .gamepad_is_active = sdl_gamepad_is_active,
        .gamepad_play_rumble = sdl_gamepad_play_rumble,
        .new_ev_listener = new_ev_listener,
        .pop_ev = pop_ev,
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


void CE_MODULE_LOAD(machine)(struct ce_api_a0 *api,
                             int reload) {
    CE_UNUSED(reload);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ce_api_a0 = api;

    _init_listener_pack(&_G.obj_listeners);

    api->register_api(CT_MACHINE_API, &a0, sizeof(a0));
    api->add_impl(CT_KERNEL_TASK_I, &machine_task, sizeof(machine_task));

    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);

    if (SDL_Init(SDL_INIT_VIDEO |
                 SDL_INIT_GAMECONTROLLER |
                 SDL_INIT_HAPTIC |
                 SDL_INIT_JOYSTICK) != 0) {
        ce_log_a0->error(LOG_WHERE, "Could not init sdl - %s", SDL_GetError());
        return; // TODO: dksandasdnask FUCK init without return ptype?????
    }

    sdl_window_init(api);
}

void CE_MODULE_UNLOAD(machine)(struct ce_api_a0 *api,
                               int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    sdl_window_shutdown();
    SDL_Quit();
}

