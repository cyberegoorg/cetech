//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/math_types.h>
#include <cetech/celib/allocator.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource/resource.h>
#include <cetech/core/module.h>
#include <cetech/celib/eventstream.inl>
#include <cetech/core/machine.h>

#include <cetech/modules/input/input.h>
#include <cetech/core/api.h>

#include "mousebtnstr.h"

IMPORT_API(machine_api_v0);
IMPORT_API(log_api_v0);


//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "mouse"


//==============================================================================
// Globals
//==============================================================================


static struct G {
    uint8_t state[MOUSE_BTN_MAX];
    uint8_t last_state[MOUSE_BTN_MAX];
    vec2f_t last_pos;
    vec2f_t last_delta_pos;
} _G = {0};

//==============================================================================
// Interface
//==============================================================================
namespace mouse {
    uint32_t button_index(const char *button_name) {
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

    const char *button_name(const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

        return _btn_to_str[button_index];
    }

    int button_state(uint32_t idx,
                     const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

        return _G.state[button_index];
    }

    int button_pressed(uint32_t idx,
                       const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

        return _G.state[button_index] && !_G.last_state[button_index];
    }

    int button_released(uint32_t idx,
                        const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < MOUSE_BTN_MAX));

        return !_G.state[button_index] && _G.last_state[button_index];
    }

    const char *axis_name(const uint32_t axis_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (axis_index >= 0) && (axis_index < MOUSE_AXIS_MAX));

        return _axis_to_str[axis_index];
    }

    uint32_t axis_index(const char *axis_name) {
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

    vec2f_t axis(uint32_t idx,
                 const uint32_t axis_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (axis_index >= 0) && (axis_index < MOUSE_AXIS_MAX));

        switch (axis_index) {
            case MOUSE_AXIS_ABSOULTE:
                return _G.last_pos;

            case MOUSE_AXIS_RELATIVE:
                return _G.last_delta_pos;

            default:
                return (vec2f_t) {0};
        }
    }

    void mouse_set_cursor_pos(vec2f_t pos) {
        //TODO: implement
    }
}

namespace mouse_module {
    static struct mouse_api_v0 api_v1 = {
            .button_index = mouse::button_index,
            .button_name = mouse::button_name,
            .button_state = mouse::button_state,
            .button_pressed = mouse::button_pressed,
            .button_released = mouse::button_released,
            .axis_index = mouse::axis_index,
            .axis_name = mouse::axis_name,
            .axis = mouse::axis,
    };

    void _init_api(struct api_v0 *api) {
        api->register_api("mouse_api_v0", &api_v1);
    }

    void _init(struct api_v0 *api) {
        GET_API(api, machine_api_v0);
        GET_API(api, log_api_v0);

        _G = {0};

        log_api_v0.debug(LOG_WHERE, "Init");
    }

    void _shutdown() {
        log_api_v0.debug(LOG_WHERE, "Shutdown");

        _G = {0};
    }


    void _update() {
        struct event_header *event = machine_api_v0.event_begin();

        memcpy(_G.last_state, _G.state, MOUSE_BTN_MAX);
        _G.last_delta_pos.x = 0;
        _G.last_delta_pos.y = 0;

        while (event != machine_api_v0.event_end()) {
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
                    log_api_v0.debug(LOG_WHERE, "mouse: %f %f",
                                     _G.last_delta_pos.x, _G.last_delta_pos.y);
                    break;

                default:
                    break;
            }

            event = machine_api_v0.event_next(event);
        }
    }


    extern "C" void *mouse_get_module_api(int api) {

        if (api == PLUGIN_EXPORT_API_ID) {
            static struct module_api_v0 module = {0};

            module.init = _init;
            module.init_api = _init_api;
            module.shutdown = _shutdown;
            module.update = _update;

            return &module;

        }

        return 0;
    }

}