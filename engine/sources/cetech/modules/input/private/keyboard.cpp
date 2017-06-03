//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/allocator.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource/resource.h>
#include <cetech/core/module.h>
#include <cetech/celib/eventstream.inl>
#include <cetech/core/machine.h>
#include <cetech/modules/input/input.h>
#include <cetech/core/api.h>

#include "keystr.h"

IMPORT_API(machine_api_v0);
IMPORT_API(log_api_v0);

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "keyboard"


//==============================================================================
// Globals
//==============================================================================

static struct G {
    uint8_t state[512];
    uint8_t last_state[512];
} _G = {0};


//==============================================================================
// Interface
//==============================================================================

namespace keyboard {
    uint32_t button_index(const char *button_name) {
        for (uint32_t i = 0; i < KEY_MAX; ++i) {
            if (!_key_to_str[i]) {
                continue;
            }

            if (strcmp(_key_to_str[i], button_name)) {
                continue;
            }

            return i;
        }

        return 0;
    }

    const char *button_name(const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < KEY_MAX));

        return _key_to_str[button_index];
    }

    int button_state(uint32_t idx,
                     const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < KEY_MAX));

        return _G.state[button_index];
    }

    int button_pressed(uint32_t idx,
                       const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < KEY_MAX));

        return _G.state[button_index] && !_G.last_state[button_index];
    }

    int button_released(uint32_t idx,
                        const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < KEY_MAX));

        return !_G.state[button_index] && _G.last_state[button_index];
    }
}

namespace keyboard_module {
    static struct keyboard_api_v0 api_v1 = {
            .button_index = keyboard::button_index,
            .button_name = keyboard::button_name,
            .button_state = keyboard::button_state,
            .button_pressed = keyboard::button_pressed,
            .button_released = keyboard::button_released,
    };

    void _init_api(struct api_v0 *api) {
        api->register_api("keyboard_api_v0", &api_v1);
    }

    void _init(struct api_v0 *api) {
        GET_API(api, machine_api_v0);
        GET_API(api, log_api_v0);

        _G = (struct G) {0};

        log_api_v0.debug(LOG_WHERE, "Init");
    }

    void _shutdown() {
        log_api_v0.debug(LOG_WHERE, "Shutdown");

        _G = (struct G) {0};
    }


    void _update() {
        struct event_header *event = machine_api_v0.event_begin();

        memcpy(_G.last_state, _G.state, 512);

        uint32_t size = 0;
        while (event != machine_api_v0.event_end()) {
            size = size + 1;

            switch (event->type) {
                case EVENT_KEYBOARD_DOWN:
                    _G.state[((struct keyboard_event *) event)->keycode] = 1;
                    break;

                case EVENT_KEYBOARD_UP:
                    _G.state[((struct keyboard_event *) event)->keycode] = 0;
                    break;

                default:
                    break;
            }

            event = machine_api_v0.event_next(event);
        }
    }


    extern "C" void *keyboard_get_module_api(int api) {

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