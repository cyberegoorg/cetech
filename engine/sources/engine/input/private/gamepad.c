//==============================================================================
// Includes
//==============================================================================

#include <celib/math/types.h>
#include <engine/input/input.h>
#include "celib/machine/machine.h"
#include "celib/string/string.h"

#include "gamepadstr.h"

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "gamepad"


//==============================================================================
// Globals
//==============================================================================


static struct G {
    int active[GAMEPAD_MAX];
    vec2f_t position[GAMEPAD_MAX][GAMEPAD_AXIX_MAX];
    int state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
    int last_state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
} _G = {0};


//==============================================================================
// Interface
//==============================================================================

int gamepad_init(int stage) {
    if (stage == 0) {
        return 1;
    }

    _G = (struct G) {0};

    log_debug(LOG_WHERE, "Init");

    for (int i = 0; i < GAMEPAD_MAX; ++i) {
        _G.active[i] = machine_gamepad_is_active(i);
    }

    return 1;
}

void gamepad_shutdown() {
    _G = (struct G) {0};

    log_debug(LOG_WHERE, "Shutdown");
}

void gamepad_process() {
    struct event_header *event = machine_event_begin();

    memory_copy(_G.last_state, _G.state, sizeof(int) * GAMEPAD_BTN_MAX * GAMEPAD_MAX);

    while (event != machine_event_end()) {
        struct gamepad_move_event *move_event = (struct gamepad_move_event *) event;
        struct gamepad_btn_event *btn_event = (struct gamepad_btn_event *) event;
        struct gamepad_device_event *device_event = (struct gamepad_device_event *) event;

        switch (event->type) {
            case EVENT_GAMEPAD_DOWN:
                _G.state[btn_event->gamepad_id][btn_event->button] = 1;
                break;

            case EVENT_GAMEPAD_UP:
                _G.state[btn_event->gamepad_id][btn_event->button] = 0;
                break;

            case EVENT_GAMEPAD_MOVE:
                _G.position[move_event->gamepad_id][move_event->axis] = move_event->position;
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

        event = machine_event_next(event);
    }
}

int gamepad_is_active(gamepad_t gamepad) {
    return _G.active[gamepad.idx];
}

u32 gamepad_button_index(const char *button_name) {
    for (u32 i = 0; i < GAMEPAD_BTN_MAX; ++i) {
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

const char *gamepad_button_name(const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _btn_to_str[button_index];
}

int gamepad_button_state(gamepad_t gamepad,
                         const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _G.state[gamepad.idx][button_index];
}

int gamepad_button_pressed(gamepad_t gamepad,
                           const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _G.state[gamepad.idx][button_index] && !_G.last_state[gamepad.idx][button_index];
}

int gamepad_button_released(gamepad_t gamepad,
                            const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return !_G.state[gamepad.idx][button_index] && _G.last_state[gamepad.idx][button_index];
}

const char *gamepad_axis_name(const u32 axis_index) {
    CEL_ASSERT(LOG_WHERE, (axis_index >= 0) && (axis_index < GAMEPAD_AXIX_MAX));

    return _axis_to_str[axis_index];
}

u32 gamepad_axis_index(const char *axis_name) {
    for (u32 i = 0; i < GAMEPAD_AXIX_MAX; ++i) {
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

vec2f_t gamepad_axis(gamepad_t gamepad,
                     const u32 axis_index) {
    CEL_ASSERT(LOG_WHERE, (axis_index >= 0) && (axis_index < GAMEPAD_AXIX_MAX));

    return _G.position[gamepad.idx][axis_index];
}

void gamepad_play_rumble(gamepad_t gamepad,
                         float strength,
                         u32 length) {
    machine_gamepad_play_rumble(gamepad.idx, strength, length);
}

