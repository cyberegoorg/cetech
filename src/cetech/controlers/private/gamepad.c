//==============================================================================
// Includes
//==============================================================================
#include <string.h>

#include <celib/api.h>
#include <celib/log.h>
#include <celib/module.h>

#include <celib/id.h>
#include <celib/macros.h>

#include "celib/memory/allocator.h"

#include <cetech/machine/machine.h>
#include <cetech/kernel/kernel.h>
#include <cetech/controlers/gamepad.h>
#include <cetech/controlers/controlers.h>
#include <celib/cdb.h>
#include <celib/math/math.h>

#include "gamepadstr.h"


//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "gamepad"


//==============================================================================
// Globals
//==============================================================================

#define GAMEPAD_MAX 8

#define _G GamepadGlobals

typedef struct gamepad_t {
    float position[GAMEPAD_AXIX_MAX][2];
    float death_zone[GAMEPAD_AXIX_MAX][2];
    int state[GAMEPAD_BTN_MAX];
    int last_state[GAMEPAD_BTN_MAX];
} gamepad_t;

static struct _G {
    int active[GAMEPAD_MAX];
    gamepad_t gamepad[GAMEPAD_MAX];

    ct_machine_ev_queue_o0 *ev_queue;
} _G = {};


//==============================================================================
// Interface
//==============================================================================

static int is_active(uint32_t idx) {
    return _G.active[idx];
}

static uint32_t button_index(const char *button_name) {
    for (uint32_t i = 0; i < GAMEPAD_BTN_MAX; ++i) {
        if (!_btn_to_str[i]) {
            continue;
        }

        if (strcmp(_btn_to_str[i], button_name)) {
            continue;
        }

        return i;
    }

    return 0;
}

static const char *button_name(const uint32_t button_index) {
    CE_ASSERT(LOG_WHERE,
              (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _btn_to_str[button_index];
}

static int button_state(uint32_t idx,
                        const uint32_t button_index) {
    CE_ASSERT(LOG_WHERE,
              (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _G.gamepad[idx].state[button_index];
}

static int button_pressed(uint32_t idx,
                          const uint32_t button_index) {
    CE_ASSERT(LOG_WHERE,
              (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _G.gamepad[idx].state[button_index] && !_G.gamepad[idx].last_state[button_index];
}

static int button_released(uint32_t idx,
                           const uint32_t button_index) {
    CE_ASSERT(LOG_WHERE,
              (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return !_G.gamepad[idx].state[button_index] && _G.gamepad[idx].last_state[button_index];
}

static const char *axis_name(const uint32_t axis_index) {
    CE_ASSERT(LOG_WHERE,
              (axis_index >= 0) && (axis_index < GAMEPAD_AXIX_MAX));

    return _axis_to_str[axis_index];
}

static uint32_t axis_index(const char *axis_name) {
    for (uint32_t i = 0; i < GAMEPAD_AXIX_MAX; ++i) {
        if (!_axis_to_str[i]) {
            continue;
        }

        if (strcmp(_axis_to_str[i], axis_name) != 0) {
            continue;
        }

        return i;
    }

    return 0;
}

static void axis(uint32_t idx,
                 const uint32_t axis_index,
                 float *value) {
    CE_ASSERT(LOG_WHERE,
              (axis_index >= 0) && (axis_index < GAMEPAD_AXIX_MAX));

    value[0] = _G.gamepad[idx].position[axis_index][0];
    value[1] = _G.gamepad[idx].position[axis_index][1];
}

static void play_rumble(uint32_t idx,
                        float strength,
                        uint32_t length) {
    ct_machine_a0->gamepad_play_rumble(idx, strength, length);
}

static void update(float dt) {
    for (int i = 0; i < GAMEPAD_MAX; ++i) {
        memcpy(_G.gamepad[i].last_state, _G.gamepad[i].state, sizeof(int) * GAMEPAD_BTN_MAX);
    }

    ct_machine_ev_t0 ev = {};
    while (ct_machine_a0->pop_ev(_G.ev_queue, &ev)) {
        switch (ev.ev_type) {
            case EVENT_GAMEPAD_DOWN: {
                gamepad_t *g = &_G.gamepad[ev.gamepad.gamepad_id];

                g->state[ev.gamepad.btn] = 1;
            }
                break;

            case EVENT_GAMEPAD_UP: {
                gamepad_t *g = &_G.gamepad[ev.gamepad.gamepad_id];

                g->state[ev.gamepad.btn] = 0;
            }
                break;

            case EVENT_GAMEPAD_MOVE: {
                gamepad_t *g = &_G.gamepad[ev.gamepad.gamepad_id];

                if (ce_fabsolute(ev.gamepad.pos.x) > g->death_zone[ev.gamepad.axis_id][0]) {
                    g->position[ev.gamepad.axis_id][0] = ev.gamepad.pos.x;
                } else {
                    g->position[ev.gamepad.axis_id][0] = 0;
                }

                if (ce_fabsolute(ev.gamepad.pos.y) > g->death_zone[ev.gamepad.axis_id][1]) {
                    g->position[ev.gamepad.axis_id][1] = ev.gamepad.pos.y;
                } else {
                    g->position[ev.gamepad.axis_id][1] = 0;
                }

            }
                break;

            case EVENT_GAMEPAD_CONNECT: {
                _G.active[ev.gamepad.btn] = 1;
            }
                break;

            case EVENT_GAMEPAD_DISCONNECT: {
                _G.active[ev.gamepad.gamepad_id] = 0;
            }
                break;
            default:
                break;
        }
    }

}

static uint64_t name() {
    return CONTROLER_GAMEPAD;
}

static struct ct_controler_i0 ct_controlers_api = {
        .name = name,
        .is_active = is_active,
        .button_index = button_index,
        .button_name = button_name,
        .button_state = button_state,
        .button_pressed = button_pressed,
        .button_released = button_released,
        .axis_index = axis_index,
        .axis_name = axis_name,
        .axis = axis,
        .play_rumble = play_rumble,
};

static struct ct_kernel_task_i0 gamepad_task = {
        .name = CT_GAMEPAD_TASK,
        .update = update,
        .update_before= CT_KERNEL_BEFORE(CT_INPUT_TASK),
        .update_after = CT_KERNEL_AFTER(CT_MACHINE_TASK),
};

void CE_MODULE_LOAD(gamepad)(struct ce_api_a0 *api,
                             int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ct_machine_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);

    _G = (struct _G) {
            .ev_queue = ct_machine_a0->new_ev_listener(),
    };

    api->add_impl(CT_CONTROLERS_I, &ct_controlers_api, sizeof(ct_controlers_api));
    api->add_impl(CT_KERNEL_TASK_I, &gamepad_task, sizeof(gamepad_task));

    for (int i = 0; i < GAMEPAD_MAX; ++i) {
        _G.active[i] = ct_machine_a0->gamepad_is_active(i);

        for (int j = 0; j < GAMEPAD_AXIX_MAX; ++j) {
            _G.gamepad[i].death_zone[j][0] = 0.1f;
            _G.gamepad[i].death_zone[j][1] = 0.1f;
        }

    }
}

void CE_MODULE_UNLOAD(gamepad)(struct ce_api_a0 *api,
                               int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _G = (struct _G) {};
}
