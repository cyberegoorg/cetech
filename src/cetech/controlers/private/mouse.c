//==============================================================================
// Includes
//==============================================================================

#include <cetech/machine/machine.h>
#include <cetech/controlers/mouse.h>
#include <string.h>
#include <corelib/hashlib.h>
#include <corelib/ebus.h>
#include <cetech/kernel/kernel.h>
#include <corelib/os.h>
#include <corelib/macros.h>
#include "corelib/allocator.h"

#include "corelib/api_system.h"

#include "mousebtnstr.h"
#include "corelib/log.h"
#include "corelib/module.h"


//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "mouse"


//==============================================================================
// Globals
//==============================================================================


#define _G MouseGlobals
static struct _G {
    uint8_t state[MOUSE_BTN_MAX];
    uint8_t last_state[MOUSE_BTN_MAX];
    float pos[3];
    float delta_pos[3];
    float wheel[3];
    float wheel_last[3];
} _G = {};

//==============================================================================
// Interface
//==============================================================================
static uint32_t button_index(const char *button_name) {
    for (uint32_t i = 0; i < MOUSE_BTN_MAX; ++i) {
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
                  (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return _btn_to_str[button_index];
}

static int button_state(uint32_t idx,
                        const uint32_t button_index) {
    CT_UNUSED(idx);
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return _G.state[button_index];
}

static int button_pressed(uint32_t idx,
                          const uint32_t button_index) {
    CT_UNUSED(idx);
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return _G.state[button_index] && !_G.last_state[button_index];
}

static int button_released(uint32_t idx,
                           const uint32_t button_index) {
    CT_UNUSED(idx);
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return !_G.state[button_index] && _G.last_state[button_index];
}

static const char *axis_name(const uint32_t axis_index) {
    CETECH_ASSERT(LOG_WHERE,
                  (axis_index >= 0) && (axis_index < MOUSE_AXIS_MAX));

    return _axis_to_str[axis_index];
}

static uint32_t axis_index(const char *axis_name) {
    for (uint32_t i = 0; i < MOUSE_AXIS_MAX; ++i) {
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
    CT_UNUSED(idx);
    CETECH_ASSERT(LOG_WHERE,
                  (axis_index >= 0) && (axis_index < MOUSE_AXIS_MAX));

    switch (axis_index) {
        case MOUSE_AXIS_ABSOULTE:
            value[0] = _G.pos[0];
            value[1] = _G.pos[1];
            return;

        case MOUSE_AXIS_RELATIVE:
            value[0] = _G.delta_pos[0];
            value[1] = _G.delta_pos[1];

            return;

        case MOUSE_AXIS_WHEEL:
            value[0] = _G.wheel[0];
            value[1] = _G.wheel[1];

            return;

        default:
            value[0] = 0.0f;
            value[1] = 0.0f;
            return;
    }
}

//    void mouse_set_cursor_pos(vec2f_t pos) {
//        //TODO: implement
//    }

static void update(uint64_t _event) {
    CT_UNUSED(_event);

    memcpy(_G.last_state, _G.state, MOUSE_BTN_MAX);
    _G.delta_pos[0] = 0;
    _G.delta_pos[1] = 0;
//    _G.wheel[0] = 0;
//    _G.wheel[1] = 0;

    uint64_t *events = ct_ebus_a0->events(MOUSE_EBUS);
    uint32_t events_n = ct_ebus_a0->event_count(MOUSE_EBUS);

    for (int i = 0; i < events_n; ++i) {
        uint64_t event = events[i];
        uint32_t button = ct_cdb_a0->read_uint64(event, CT_ID64_0("button"), 0);

        switch (ct_cdb_a0->type(event)) {
            case EVENT_MOUSE_DOWN:
                _G.state[button] = 1;
                break;

            case EVENT_MOUSE_UP:
                _G.state[button] = 0;
                break;

            case EVENT_MOUSE_MOVE: {
                float pos[3];

                ct_cdb_a0->read_vec3(event, CT_ID64_0("position"), pos);

                _G.delta_pos[0] = pos[0] - _G.pos[0];
                _G.delta_pos[1] = pos[1] - _G.pos[1];

                _G.pos[0] = pos[0];
                _G.pos[1] = pos[1];
            }
                break;

            case EVENT_MOUSE_WHEEL: {
                float pos[3];

                ct_cdb_a0->read_vec3(event, CT_ID64_0("position"), pos);

                _G.wheel[0] += pos[0];// - _G.wheel_last[0];
                _G.wheel[1] += pos[1];// - _G.wheel_last[1];

                _G.wheel_last[0] = pos[0];
                _G.wheel_last[1] = pos[1];
            }
                break;

            default:
                break;
        }
    }
}

static struct ct_mouse_a0 a0 = {
        .button_index = button_index,
        .button_name = button_name,
        .button_state = button_state,
        .button_pressed = button_pressed,
        .button_released = button_released,
        .axis_index = axis_index,
        .axis_name = axis_name,
        .axis = axis,
};

struct ct_mouse_a0 *ct_mouse_a0 = &a0;

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_mouse_a0", &a0);
}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {};

    ct_ebus_a0->create_ebus(MOUSE_EBUS_NAME, MOUSE_EBUS);

    ct_ebus_a0->connect(KERNEL_EBUS, KERNEL_UPDATE_EVENT, update, 0);

    ct_log_a0->debug(LOG_WHERE, "Init");
}

static void _shutdown() {
    ct_log_a0->debug(LOG_WHERE, "Shutdown");

    ct_ebus_a0->disconnect(KERNEL_EBUS, KERNEL_UPDATE_EVENT, update);

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        mouse,
        {
            CETECH_GET_API(api, ct_machine_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_cdb_a0);

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