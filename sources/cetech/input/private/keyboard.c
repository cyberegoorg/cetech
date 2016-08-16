#include <celib/types.h>
#include <cetech/machine/machine.h>
#include <celib/memory/memory.h>
#include <celib/string/string.h>
#include <celib/errors/errors.h>

#include "../input.h"

#include "keystr.h"

#define LOG_WHERE "keyboard"

static struct G {
    u8 state[512];
    u8 last_state[512];
} _G = {0};

int keyboard_init() {
    _G = (struct G) {0};
    return 1;
}

void keyboard_shutdow() {
    _G = (struct G) {0};
}

void keyboard_process() {
    struct event_header *event = machine_event_begin();

    memory_copy(_G.last_state, _G.state, 512);

    u32 size = 0;
    while (event != machine_event_end()) {
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

        event = machine_event_next(event);
    }

}

u32 keyboard_button_index(const char *button_name) {
    for (u32 i = 0; i < KEY_MAX; ++i) {
        if (!_key_to_str[i]) {
            continue;
        }

        if (str_compare(_key_to_str[i], button_name)) {
            continue;
        }

        return i;
    }

    return 0;
}

const char *keyboard_button_name(const u32 button_index) {
    CE_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return _key_to_str[button_index];
}

int keyboard_button_state(const u32 button_index) {
    CE_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return _G.state[button_index];
}

int keyboard_button_pressed(const u32 button_index) {
    CE_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return _G.state[button_index] && !_G.last_state[button_index];
}

int keyboard_button_released(const u32 button_index) {
    CE_ASSERT(LOG_WHERE, (button_index >= 0) && (button_index < KEY_MAX));

    return !_G.state[button_index] && _G.last_state[button_index];
}

