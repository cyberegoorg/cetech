//==============================================================================
// Includes
//==============================================================================

#include <celib/math/types.h>
#include "engine/machine/machine.h"
#include "celib/string/string.h"
#include "celib/math/vec2f.h"

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
    vec2f_t last_pos;
    vec2f_t last_delta_pos;
} _G = {0};


//==============================================================================
// Interface
//==============================================================================


int mouse_init() {
    _G = (struct G) {0};

    log_debug(LOG_WHERE, "Init");

    return 1;
}

void mouse_shutdown() {
    _G = (struct G) {0};

    log_debug(LOG_WHERE, "Shutdown");
}

void mouse_process() {
    struct event_header *event = machine_event_begin();

    memory_copy(_G.last_state, _G.state, MOUSE_BTN_MAX);
    _G.last_delta_pos.x = 0;
    _G.last_delta_pos.y = 0;

    while (event != machine_event_end()) {
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

                _G.last_delta_pos.x = _G.last_pos.x - move_event->pos.x;
                _G.last_delta_pos.y = _G.last_pos.y - move_event->pos.y;

                _G.last_pos.x = move_event->pos.x;
                _G.last_pos.y = move_event->pos.y;
                break;

            default:
                break;
        }

        event = machine_event_next(event);
    }
}

u32 mouse_button_index(const char *button_name) {
    for (u32 i = 0; i < MOUSE_BTN_MAX; ++i) {
        if (!_btn_to_str[i]) {
            continue;
        }

        if (str_compare(_btn_to_str[i], button_name)) {
            continue;
        }

        return i;
    }

    return 0;
}

const char *mouse_button_name(const u32 button_index) {
    CE_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return _btn_to_str[button_index];
}

int mouse_button_state(const u32 button_index) {
    CE_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return _G.state[button_index];
}

int mouse_button_pressed(const u32 button_index) {
    CE_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return _G.state[button_index] && !_G.last_state[button_index];
}

int mouse_button_released(const u32 button_index) {
    CE_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

    return !_G.state[button_index] && _G.last_state[button_index];
}

const char *mouse_axis_name(const u32 axis_index) {
    CE_ASSERT(LOG_WHERE, (axis_index >= 0) && (axis_index < MOUSE_AXIS_MAX));

    return _axis_to_str[axis_index];
}

u32 mouse_axis_index(const char *axis_name) {
    for (u32 i = 0; i < MOUSE_AXIS_MAX; ++i) {
        if (!_axis_to_str[i]) {
            continue;
        }

        if (str_compare(_axis_to_str[i], axis_name) != 0) {
            continue;
        }

        return i;
    }

    return 0;
}

void mouse_axis(const u32 axis_index,
                vec2f_t *position) {
    CE_ASSERT(LOG_WHERE, (axis_index >= 0) && (axis_index < MOUSE_AXIS_MAX));

    switch (axis_index) {
        case MOUSE_AXIS_ABSOULTE:
            vec2f_move(position, &_G.last_pos);
            return;

        case MOUSE_AXIS_RELATIVE:
            vec2f_move(position, &_G.last_delta_pos);
            return;

        default:
            return;
    }
}
