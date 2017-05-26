//==============================================================================
// Includes
//==============================================================================

#include <cetech/core/allocator.h>
#include <cetech/kernel/config.h>
#include <cetech/modules/resource/resource.h>
#include <cetech/kernel/module.h>
#include <cetech/core/eventstream.inl>
#include <cetech/kernel/machine.h>
#include <cetech/kernel/input.h>
#include <cetech/kernel/string.h>
#include <cetech/kernel/api.h>

#include "keystr.h"

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

IMPORT_API(machine_api_v0);


static void _update() {
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


//==============================================================================
// Interface
//==============================================================================

uint32_t keyboard_button_index(const char *button_name) {
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

const char *keyboard_button_name(const uint32_t button_index) {
    CETECH_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return _key_to_str[button_index];
}

int keyboard_button_state(uint32_t idx,
                          const uint32_t button_index) {
    CETECH_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return _G.state[button_index];
}

int keyboard_button_pressed(uint32_t idx,
                            const uint32_t button_index) {
    CETECH_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return _G.state[button_index] && !_G.last_state[button_index];
}

int keyboard_button_released(uint32_t idx,
                             const uint32_t button_index) {
    CETECH_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return !_G.state[button_index] && _G.last_state[button_index];
}

static void _init_api(struct api_v0* api){
    static struct keyboard_api_v0 api_v1 = {
            .button_index = keyboard_button_index,
            .button_name = keyboard_button_name,
            .button_state = keyboard_button_state,
            .button_pressed = keyboard_button_pressed,
            .button_released = keyboard_button_released,
    };

    api->register_api("keyboard_api_v0", &api_v1);
}

static void _init( struct api_v0* api) {
    USE_API(api, machine_api_v0);



    _G = (struct G) {0};

    log_debug(LOG_WHERE, "Init");
}

static void _shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    _G = (struct G) {0};
}


void *keyboard_get_module_api(int api) {

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