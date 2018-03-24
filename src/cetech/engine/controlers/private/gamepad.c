//==============================================================================
// Includes
//==============================================================================

#include <cetech/core/api/api_system.h>
#include <cetech/core/log/log.h>
#include <cetech/engine/machine/machine.h>
#include <cetech/core/os/errors.h>
#include <cetech/engine/controlers/gamepad.h>
#include <cetech/core/module/module.h>
#include <cetech/engine/application/application.h>
#include <string.h>
#include <cetech/core/ebus/ebus.h>
#include <cetech/core/hashlib/hashlib.h>
#include "cetech/core/memory/allocator.h"
#include "gamepadstr.h"

CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_machine_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_ebus_a0);

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "gamepad"


//==============================================================================
// Globals
//==============================================================================

#define _G GamepadGlobals
static struct _G {
    int active[GAMEPAD_MAX];
    float position[GAMEPAD_MAX][GAMEPAD_AXIX_MAX][2];
    int state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
    int last_state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
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
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _btn_to_str[button_index];
}

static int button_state(uint32_t idx,
                        const uint32_t button_index) {
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _G.state[idx][button_index];
}

static int button_pressed(uint32_t idx,
                          const uint32_t button_index) {
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _G.state[idx][button_index] && !_G.last_state[idx][button_index];
}

static int button_released(uint32_t idx,
                           const uint32_t button_index) {
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return !_G.state[idx][button_index] && _G.last_state[idx][button_index];
}

static const char *axis_name(const uint32_t axis_index) {
    CETECH_ASSERT(LOG_WHERE,
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
    CETECH_ASSERT(LOG_WHERE,
                  (axis_index >= 0) && (axis_index < GAMEPAD_AXIX_MAX));

    value[0] = _G.position[idx][axis_index][0];
    value[1] = _G.position[idx][axis_index][1];
}

static void play_rumble(uint32_t idx,
                        float strength,
                        uint32_t length) {
    ct_machine_a0.gamepad_play_rumble(idx, strength, length);
}

static void update(uint32_t bus_name, void *_event) {


    memcpy(_G.last_state, _G.state,
           sizeof(int) * GAMEPAD_BTN_MAX * GAMEPAD_MAX);

    void* event = ct_ebus_a0.first_event(GAMEPAD_EBUS);
    struct ebus_header_t *header;
    while (event) {
        header = ct_ebus_a0.event_header(event);

        struct ct_gamepad_move_event *move_event = (struct ct_gamepad_move_event *) event;
        struct ct_gamepad_btn_event *btn_event = (struct ct_gamepad_btn_event *) event;
        struct ct_gamepad_device_event *device_event = (struct ct_gamepad_device_event *) event;

        switch (header->type) {
            case EVENT_GAMEPAD_DOWN:
                _G.state[btn_event->gamepad_id][btn_event->button] = 1;
                break;

            case EVENT_GAMEPAD_UP:
                _G.state[btn_event->gamepad_id][btn_event->button] = 0;
                break;

            case EVENT_GAMEPAD_MOVE:
                _G.position[move_event->gamepad_id][move_event->axis][0] = move_event->position[0];
                _G.position[move_event->gamepad_id][move_event->axis][1] = move_event->position[1];
                break;

            case EVENT_GAMEPAD_CONNECT:
                _G.active[device_event->gamepad_id] = 1;
                break;

            case EVENT_GAMEPAD_DISCONNECT:
                _G.active[device_event->gamepad_id] = 0;
                break;


            default:
                break;
        }

        event = ct_ebus_a0.next_event(GAMEPAD_EBUS, event);
    }
}

static struct ct_gamepad_a0 a0 = {
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

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_gamepad_a0", &a0);
}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);
    _G = (struct _G) {};

    ct_ebus_a0.connect(APPLICATION_EBUS,
                                APP_UPDATE_EVENT, update, 0);

    ct_ebus_a0.create_ebus(GAMEPAD_EBUS_NAME, GAMEPAD_EBUS);

    ct_log_a0.debug(LOG_WHERE, "Init");

    for (int i = 0; i < GAMEPAD_MAX; ++i) {
        _G.active[i] = ct_machine_a0.gamepad_is_active(i);
    }
}

static void _shutdown() {
    ct_log_a0.debug(LOG_WHERE, "Shutdown");

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        gamepad,
        {
            CETECH_GET_API(api, ct_machine_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_app_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            _shutdown();

        }
)