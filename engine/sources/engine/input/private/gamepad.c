//==============================================================================
// Includes
//==============================================================================

#include <celib/math/types.h>
#include <engine/input/gamepad.h>
#include <engine/plugin/plugin_api.h>
#include "engine/machine/machine.h"
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
    cel_vec2f_t position[GAMEPAD_MAX][GAMEPAD_AXIX_MAX];
    int state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
    int last_state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
} _G = {0};

static struct MachineApiV0 MachineApiV0 = {0};


static void _init(get_api_fce_t get_engine_api) {
    _G = (struct G) {0};

    MachineApiV0 = *(struct MachineApiV0*) get_engine_api(MACHINE_API_ID, 0);

    log_debug(LOG_WHERE, "Init");

    for (int i = 0; i < GAMEPAD_MAX; ++i) {
        _G.active[i] = MachineApiV0.gamepad_is_active(i);
    }

}

static void _shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    _G = (struct G) {0};
}

static void _update() {
    struct event_header *event = MachineApiV0.event_begin();

    memory_copy(_G.last_state, _G.state, sizeof(int) * GAMEPAD_BTN_MAX * GAMEPAD_MAX);

    while (event != MachineApiV0.event_end()) {
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

        event = MachineApiV0.event_next(event);
    }
}





//==============================================================================
// Interface
//==============================================================================


int gamepad_is_active(u32 idx) {
    return _G.active[idx];
}

u32 gamepad_button_index(const char *button_name) {
    for (u32 i = 0; i < GAMEPAD_BTN_MAX; ++i) {
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

const char *gamepad_button_name(const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _btn_to_str[button_index];
}

int gamepad_button_state(u32 idx,
                         const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _G.state[idx][button_index];
}

int gamepad_button_pressed(u32 idx,
                           const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return _G.state[idx][button_index] && !_G.last_state[idx][button_index];
}

int gamepad_button_released(u32 idx,
                            const u32 button_index) {
    CEL_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

    return !_G.state[idx][button_index] && _G.last_state[idx][button_index];
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

        if (cel_strcmp(_axis_to_str[i], axis_name) != 0) {
            continue;
        }

        return i;
    }

    return 0;
}

cel_vec2f_t gamepad_axis(u32 idx,
                         const u32 axis_index) {
    CEL_ASSERT(LOG_WHERE, (axis_index >= 0) && (axis_index < GAMEPAD_AXIX_MAX));

    return _G.position[idx][axis_index];
}

void gamepad_play_rumble(u32 idx,
                         float strength,
                         u32 length) {
    MachineApiV0.gamepad_play_rumble(idx, strength, length);
}

void *gamepad_get_plugin_api(int api,
                             int version) {

    if (api == PLUGIN_EXPORT_API_ID && version == 0) {
        static struct plugin_api_v0 plugin = {0};

        plugin.init = _init;
        plugin.shutdown = _shutdown;
        plugin.update = _update;

        return &plugin;

    } else if (api == GAMEPAD_API_ID && version == 0) {
        static struct GamepadApiV0 api_v1 = {
                .is_active = gamepad_is_active,
                .button_index = gamepad_button_index,
                .button_name = gamepad_button_name,
                .button_state = gamepad_button_state,
                .button_pressed = gamepad_button_pressed,
                .button_released = gamepad_button_released,
                .axis_index = gamepad_axis_index,
                .axis_name = gamepad_axis_name,
                .axis = gamepad_axis,
                .play_rumble = gamepad_play_rumble,
        };

        return &api_v1;
    }

    return 0;
}