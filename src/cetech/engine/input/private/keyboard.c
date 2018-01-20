//==============================================================================
// Includes
//==============================================================================

#include <cetech/core/api_system.h>
#include <cetech/engine/machine/machine.h>
#include <cetech/core/log.h>
#include <cetech/core/errors.h>
#include <cetech/engine/input/input.h>
#include <cetech/core/module.h>
#include <cetech/engine/application/application.h>
#include <string.h>
#include "cetech/core/allocator.h"
#include "keystr.h"

CETECH_DECL_API(ct_machine_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_app_a0);

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
    char text[32];
} _G = {};


//==============================================================================
// Interface
//==============================================================================

static uint32_t button_index(const char *button_name) {
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

static const char *button_name(const uint32_t button_index) {
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < KEY_MAX));

    return _key_to_str[button_index];
}

static int button_state(uint32_t idx,
                        const uint32_t button_index) {
    CT_UNUSED(idx);
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < KEY_MAX));

    return _G.state[button_index];
}

static int button_pressed(uint32_t idx,
                          const uint32_t button_index) {
    CT_UNUSED(idx);
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < KEY_MAX));

    return _G.state[button_index] && !_G.last_state[button_index];
}

static int button_released(uint32_t idx,
                           const uint32_t button_index) {
    CT_UNUSED(idx);
    CETECH_ASSERT(LOG_WHERE,
                  (button_index >= 0) && (button_index < KEY_MAX));

    return !_G.state[button_index] && _G.last_state[button_index];
}

static void _update(float dt) {
    CT_UNUSED(dt);

    struct ct_event_header *event = ct_machine_a0.event_begin();

    memcpy(_G.last_state, _G.state, 512);
    memset(_G.text, 0, sizeof(_G.text));

    uint32_t size = 0;
    while (event != ct_machine_a0.event_end()) {
        size = size + 1;

        switch (event->type) {
            case EVENT_KEYBOARD_DOWN:
                _G.state[((struct ct_keyboard_event *) event)->keycode] = 1;
                break;

            case EVENT_KEYBOARD_UP:
                _G.state[((struct ct_keyboard_event *) event)->keycode] = 0;
                break;

            case EVENT_KEYBOARD_TEXT: {
                struct ct_keyboard_text_event *ev = (struct ct_keyboard_text_event *) event;
                memcpy(_G.text, ev->text, sizeof(ev->text));
                break;
            }


            default:
                break;
        }

        event = ct_machine_a0.event_next(event);
    }
}

static char *text(uint32_t idx) {
    return _G.text;
}

static struct ct_keyboard_a0 a0 = {
        .button_index = button_index,
        .button_name = button_name,
        .button_state = button_state,
        .button_pressed = button_pressed,
        .button_released = button_released,
        .text = text,
};

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_keyboard_a0", &a0);
}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);


    _G = (struct G) {};

    ct_app_a0.register_on_update(_update);


    ct_log_a0.debug(LOG_WHERE, "Init");
}

static void _shutdown() {
    ct_log_a0.debug(LOG_WHERE, "Shutdown");

    _G = (struct G) {};
}

CETECH_MODULE_DEF(
        keyboard,
        {
            CETECH_GET_API(api, ct_machine_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_app_a0);
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