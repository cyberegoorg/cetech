//==============================================================================
// Includes
//==============================================================================

#include <celib/math/types.h>
#include <engine/module/module.h>
#include "engine/machine/machine.h"

#include "engine/input/input.h"

#include "mousebtnstr.h"

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "mouse"


//==============================================================================
// Globals
//==============================================================================


static struct G {
    u8 state[MOUSE_BTN_MAX];
    u8 last_state[MOUSE_BTN_MAX];
    cel_vec2f_t last_pos;
    cel_vec2f_t last_delta_pos;
} _G = {0};


IMPORT_API(MachineApi, 0);

static void _init(get_api_fce_t get_engine_api) {
    INIT_API(MachineApi, MACHINE_API_ID, 0);

    _G = (struct G) {0};

    log_debug(LOG_WHERE, "Init");
}

static void _shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    _G = (struct G) {0};
}

static void _update() {
    struct event_header *event = MachineApiV0.event_begin();

    memory_copy(_G.last_state, _G.state, MOUSE_BTN_MAX);
//    _G.last_delta_pos.x = 0;
//    _G.last_delta_pos.y = 0;

    while (event != MachineApiV0.event_end()) {
        struct mouse_move_event *move_event;

        switch (event->type) {
            case EVENT_MOUSE_DOWN:
                _G.state[((struct mouse_event *) event)->button] = 1;
                break;

            case EVENT_MOUSE_UP:
                _G.state[((struct mouse_event *) event)->button] = 0;
                break;

            case EVENT_MOUSE_MOVE:
                move_event = ((struct mouse_move_event *) event);

                _G.last_delta_pos.x = move_event->pos.x - _G.last_pos.x;
                _G.last_delta_pos.y = move_event->pos.y - _G.last_pos.y;

                _G.last_pos.x = move_event->pos.x;
                _G.last_pos.y = move_event->pos.y;

                break;

            default:
                break;
        }

        event = MachineApiV0.event_next(event);
    }
}





//==============================================================================
// Interface
//==============================================================================

u32 mouse_button_index(const char *button_name) {
    for (u32 i = 0; i < MOUSE_BTN_MAX; ++i) {
        if (!_btn_to_str[i]) {
            continue;
        }

        if (cel_strcmp(_btn_to_str[i], button_name)) {
            continue;
        }

        return i;
    }

    return 0;
}

const char *mouse_button_name(const u32 button_index) {
    CEL_ASSERT(LOG_WHERE,
               (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return _btn_to_str[button_index];
}

int mouse_button_state(u32 idx,
                       const u32 button_index) {
    CEL_ASSERT(LOG_WHERE,
               (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return _G.state[button_index];
}

int mouse_button_pressed(u32 idx,
                         const u32 button_index) {
    CEL_ASSERT(LOG_WHERE,
               (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return _G.state[button_index] && !_G.last_state[button_index];
}

int mouse_button_released(u32 idx,
                          const u32 button_index) {
    CEL_ASSERT(LOG_WHERE,
               (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return !_G.state[button_index] && _G.last_state[button_index];
}

const char *mouse_axis_name(const u32 axis_index) {
    CEL_ASSERT(LOG_WHERE, (axis_index >= 0) && (axis_index < MOUSE_AXIS_MAX));

    return _axis_to_str[axis_index];
}

u32 mouse_axis_index(const char *axis_name) {
    for (u32 i = 0; i < MOUSE_AXIS_MAX; ++i) {
        if (!_axis_to_str[i]) {
            continue;
        }

        if (cel_strcmp(_axis_to_str[i], axis_name) != 0) {
            continue;
        }

        return i;
    }

    return 0;
}

cel_vec2f_t mouse_axis(u32 idx,
                       const u32 axis_index) {
    CEL_ASSERT(LOG_WHERE, (axis_index >= 0) && (axis_index < MOUSE_AXIS_MAX));

    switch (axis_index) {
        case MOUSE_AXIS_ABSOULTE:
            return _G.last_pos;

        case MOUSE_AXIS_RELATIVE:
            return _G.last_delta_pos;

        default:
            return (cel_vec2f_t) {0};
    }
}

void mouse_set_cursor_pos(cel_vec2f_t pos) {

}

void *mouse_get_module_api(int api,
                           int version) {

    if (api == PLUGIN_EXPORT_API_ID && version == 0) {
        static struct module_api_v0 module = {0};

        module.init = _init;
        module.shutdown = _shutdown;
        module.update = _update;

        return &module;

    } else if (api == MOUSE_API_ID && version == 0) {
        static struct MouseApiV0 api_v1 = {
                .button_index = mouse_button_index,
                .button_name = mouse_button_name,
                .button_state = mouse_button_state,
                .button_pressed = mouse_button_pressed,
                .button_released = mouse_button_released,
                .axis_index = mouse_axis_index,
                .axis_name = mouse_axis_name,
                .axis = mouse_axis,
        };

        return &api_v1;
    }

    return 0;
}
