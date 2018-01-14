//==============================================================================
// Includes
//==============================================================================

#include <cetech/machine/machine.h>
#include <cetech/input/input.h>
#include <cetech/application/application.h>
#include <string.h>
#include "celib/allocator.h"

#include "cetech/api/api_system.h"

#include "mousebtnstr.h"
#include "cetech/log/log.h"
#include "cetech/os/errors.h"
#include "cetech/module/module.h"

CETECH_DECL_API(ct_machine_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_app_a0);


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
    float pos[2];
    float delta_pos[2];
    float wheel[2];
    float wheel_last[2];
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
    CEL_UNUSED(idx);
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return _G.state[button_index];
}

static int button_pressed(uint32_t idx,
                          const uint32_t button_index) {
    CEL_UNUSED(idx);
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return _G.state[button_index] && !_G.last_state[button_index];
}

static int button_released(uint32_t idx,
                           const uint32_t button_index) {
    CEL_UNUSED(idx);
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
    CEL_UNUSED(idx);
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

static void update(float dt) {
    CEL_UNUSED(dt);
    struct ct_event_header *event = ct_machine_a0.event_begin();

    memcpy(_G.last_state, _G.state, MOUSE_BTN_MAX);
    _G.delta_pos[0] = 0;
    _G.delta_pos[1] = 0;
    _G.wheel[0] = 0;
    _G.wheel[1] = 0;

    while (event != ct_machine_a0.event_end()) {
        struct ct_mouse_move_event *move_event;

        switch (event->type) {
            case EVENT_MOUSE_DOWN:
                _G.state[((struct ct_mouse_event *) event)->button] = 1;
                break;

            case EVENT_MOUSE_UP:
                _G.state[((struct ct_mouse_event *) event)->button] = 0;
                break;

            case EVENT_MOUSE_MOVE:
                move_event = ((struct ct_mouse_move_event *) event);

                _G.delta_pos[0] = move_event->pos[0] - _G.pos[0];
                _G.delta_pos[1] = move_event->pos[1] - _G.pos[1];

                _G.pos[0] = move_event->pos[0];
                _G.pos[1] = move_event->pos[1];

                break;

            case EVENT_MOUSE_WHEEL: {
                struct ct_mouse_wheel_event *ev = ((struct ct_mouse_wheel_event *) event);
                _G.wheel[0] = ev->pos[0];// - _G.wheel_last[0];
                _G.wheel[1] = ev->pos[1];// - _G.wheel_last[1];

                _G.wheel_last[0] = ev->pos[0];
                _G.wheel_last[1] = ev->pos[1];
            }

            default:
                break;
        }

        event = ct_machine_a0.event_next(event);
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

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_mouse_a0", &a0);
}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {};

    ct_app_a0.register_on_update(update);

    ct_log_a0.debug(LOG_WHERE, "Init");
}

static void _shutdown() {
    ct_log_a0.debug(LOG_WHERE, "Shutdown");

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        mouse,
        {
            CETECH_GET_API(api, ct_machine_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_app_a0);

        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            _shutdown();

        }
)