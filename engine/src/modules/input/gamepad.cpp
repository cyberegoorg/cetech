//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/math_types.h>
#include <cetech/celib/allocator.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource.h>
#include <cetech/celib/eventstream.inl>
#include <cetech/modules/input.h>
#include <cetech/core/module.h>
#include <cetech/core/machine.h>
#include <cetech/core/api.h>

#include "gamepadstr.h"
#include <cetech/core/log.h>
#include <cetech/core/errors.h>

IMPORT_API(log_api_v0)
IMPORT_API(machine_api_v0);

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "gamepad"


//==============================================================================
// Globals
//==============================================================================

namespace {
static struct GamepadGlobals {
    int active[GAMEPAD_MAX];
    vec2f_t position[GAMEPAD_MAX][GAMEPAD_AXIX_MAX];
    int state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
    int last_state[GAMEPAD_MAX][GAMEPAD_BTN_MAX];
} _G = {0};
}


//==============================================================================
// Interface
//==============================================================================

namespace gamepad {

    int is_active(uint32_t idx) {
        return _G.active[idx];
    }

    uint32_t button_index(const char *button_name) {
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

    const char *button_name(const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

        return _btn_to_str[button_index];
    }

    int button_state(uint32_t idx,
                     const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

        return _G.state[idx][button_index];
    }

    int button_pressed(uint32_t idx,
                       const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

        return _G.state[idx][button_index] && !_G.last_state[idx][button_index];
    }

    int button_released(uint32_t idx,
                        const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < GAMEPAD_BTN_MAX));

        return !_G.state[idx][button_index] && _G.last_state[idx][button_index];
    }

    const char *axis_name(const uint32_t axis_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (axis_index >= 0) && (axis_index < GAMEPAD_AXIX_MAX));

        return _axis_to_str[axis_index];
    }

    uint32_t axis_index(const char *axis_name) {
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

    vec2f_t axis(uint32_t idx,
                 const uint32_t axis_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (axis_index >= 0) && (axis_index < GAMEPAD_AXIX_MAX));

        return _G.position[idx][axis_index];
    }

    void play_rumble(uint32_t idx,
                     float strength,
                     uint32_t length) {
        machine_api_v0.gamepad_play_rumble(idx, strength, length);
    }

}

namespace gamepad_module {

    static struct gamepad_api_v0 api_v1 = {
            .is_active = gamepad::is_active,
            .button_index = gamepad::button_index,
            .button_name = gamepad::button_name,
            .button_state = gamepad::button_state,
            .button_pressed = gamepad::button_pressed,
            .button_released = gamepad::button_released,
            .axis_index = gamepad::axis_index,
            .axis_name = gamepad::axis_name,
            .axis = gamepad::axis,
            .play_rumble = gamepad::play_rumble,
    };

    static void _init_api(struct api_v0 *api) {
        api->register_api("gamepad_api_v0", &api_v1);
    }

    static void _init(struct api_v0 *api) {
        GET_API(api, machine_api_v0);
        GET_API(api, log_api_v0);


        _G = {0};

        log_api_v0.debug(LOG_WHERE, "Init");

        for (int i = 0; i < GAMEPAD_MAX; ++i) {
            _G.active[i] = machine_api_v0.gamepad_is_active(i);
        }
    }

    static void _shutdown() {
        log_api_v0.debug(LOG_WHERE, "Shutdown");

        _G = {0};
    }


    static void _update() {
        struct event_header *event = machine_api_v0.event_begin();

        memcpy(_G.last_state, _G.state,
               sizeof(int) * GAMEPAD_BTN_MAX * GAMEPAD_MAX);

        while (event != machine_api_v0.event_end()) {
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

            event = machine_api_v0.event_next(event);
        }
    }

    extern "C" void *gamepad_get_module_api(int api) {

        if (api == PLUGIN_EXPORT_API_ID) {
            static struct module_export_api_v0 module = {0};

            module.init = _init;
            module.init_api = _init_api;
            module.shutdown = _shutdown;
            module.update = _update;

            return &module;

        }
        return 0;
    }
};