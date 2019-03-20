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
static struct _G {
    int active[GAMEPAD_MAX];
    float position[GAMEPAD_MAX][GAMEPAD_AXIX_MAX][2];
    int state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
    int last_state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
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

    return _G.state[idx][button_index];
}

static int button_pressed(uint32_t idx,
                          const uint32_t button_index) {
    CE_ASSERT(LOG_WHERE,
              (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _G.state[idx][button_index] && !_G.last_state[idx][button_index];
}

static int button_released(uint32_t idx,
                           const uint32_t button_index) {
    CE_ASSERT(LOG_WHERE,
              (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return !_G.state[idx][button_index] && _G.last_state[idx][button_index];
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

    value[0] = _G.position[idx][axis_index][0];
    value[1] = _G.position[idx][axis_index][1];
}

static void play_rumble(uint32_t idx,
                        float strength,
                        uint32_t length) {
    ct_machine_a0->gamepad_play_rumble(idx, strength, length);
}

static void update(float dt) {
    memcpy(_G.last_state, _G.state, sizeof(int) * GAMEPAD_BTN_MAX * GAMEPAD_MAX);


    ct_machine_ev_t0 ev = {};
    while (ct_machine_a0->pop_ev(_G.ev_queue, &ev)) {
        switch (ev.ev_type) {
                case EVENT_GAMEPAD_DOWN:
                    _G.state[ev.gamepad.gamepad_id][ev.gamepad.btn] = 1;
                    break;

                case EVENT_GAMEPAD_UP:
                    _G.state[ev.gamepad.gamepad_id][ev.gamepad.btn] = 0;
                    break;

                case EVENT_GAMEPAD_MOVE:
                    _G.position[ev.gamepad.gamepad_id][ev.gamepad.axis_id][0] = ev.gamepad.pos.x;
                    _G.position[ev.gamepad.gamepad_id][ev.gamepad.axis_id][1] = ev.gamepad.pos.y;
                    break;

                case EVENT_GAMEPAD_CONNECT:
                    _G.active[ev.gamepad.gamepad_id] = 1;
                    break;

                case EVENT_GAMEPAD_DISCONNECT:
                    _G.active[ev.gamepad.gamepad_id] = 0;
                    break;
                default:
                    break;
        }
    }

}

static uint64_t name() {
    return CONTROLER_GAMEPAD;
}

static struct ct_controlers_i0 ct_controlers_api = {
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

static uint64_t task_name() {
    return CT_GAMEPAD_TASK;
}

static uint64_t *update_before(uint64_t *n) {
    static uint64_t a[] = {
            CT_INPUT_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}

static uint64_t *update_after(uint64_t *n) {
    static uint64_t a[] = {
            CT_MACHINE_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}


static struct ct_kernel_task_i0 gamepad_task = {
        .name = task_name,
        .update = update,
        .update_before= update_before,
        .update_after = update_after,
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

    api->register_api(CONTROLERS_I, &ct_controlers_api, sizeof(ct_controlers_api));
    api->register_api(KERNEL_TASK_INTERFACE, &gamepad_task, sizeof(gamepad_task));

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
