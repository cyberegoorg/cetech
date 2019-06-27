//==============================================================================
// Includes
//==============================================================================

#include <celib/api.h>
#include <cetech/machine/machine.h>
#include <celib/log.h>

#include <cetech/controlers/keyboard.h>
#include <celib/module.h>
#include <string.h>
#include <celib/id.h>

#include <cetech/kernel/kernel.h>
#include <celib/macros.h>
#include "celib/memory/allocator.h"
#include "keystr.h"
#include <cetech/controlers/controlers.h>
#include <celib/cdb.h>


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
    ct_machine_ev_queue_o0 *ev_queue;
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
    CE_ASSERT(LOG_WHERE,
              (button_index >= 0) && (button_index < KEY_MAX));

    return _key_to_str[button_index];
}

static int button_state(uint32_t idx,
                        const uint32_t button_index) {
    CE_UNUSED(idx);
    CE_ASSERT(LOG_WHERE,
              (button_index >= 0) && (button_index < KEY_MAX));

    return _G.state[button_index];
}

static int button_pressed(uint32_t idx,
                          const uint32_t button_index) {
    CE_UNUSED(idx);
    CE_ASSERT(LOG_WHERE,
              (button_index >= 0) && (button_index < KEY_MAX));

    return _G.state[button_index] && !_G.last_state[button_index];
}

static int button_released(uint32_t idx,
                           const uint32_t button_index) {
    CE_UNUSED(idx);
    CE_ASSERT(LOG_WHERE,
              (button_index >= 0) && (button_index < KEY_MAX));

    return !_G.state[button_index] && _G.last_state[button_index];
}

static void update(float dt) {
    memcpy(_G.last_state, _G.state, 512);
    memset(_G.text, 0, sizeof(_G.text));

    ct_machine_ev_t0 ev = {};
    while (ct_machine_a0->pop_ev(_G.ev_queue, &ev)) {
        switch (ev.ev_type) {
            case EVENT_KEYBOARD_DOWN:
                _G.state[ev.key.keycode] = 1;
                break;

            case EVENT_KEYBOARD_UP:
                _G.state[ev.key.keycode] = 0;
                break;

            case EVENT_KEYBOARD_TEXT: {
                const char *str = ev.key.text;
                memcpy(_G.text, str, strlen(str));
                break;
            }

            default:
                break;
        }
    }
}

static char *text(uint32_t idx) {
    CE_UNUSED(idx);

    return _G.text;
}

static uint64_t name() {
    return CONTROLER_KEYBOARD;
}

static struct ct_controler_i0 ct_controlers_api = {
        .name = name,
        .button_index = button_index,
        .button_name = button_name,
        .button_state = button_state,
        .button_pressed = button_pressed,
        .button_released = button_released,
        .text = text,
};


static uint64_t task_name() {
    return CT_KEYBOARD_TASK;
}

static uint64_t *update_before(uint64_t *n) {
    static uint64_t a[] = {
            CT_INPUT_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}

static uint64_t *update_after(uint64_t *n) {
    static uint64_t a[] = {
            CT_MACHINE_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}


static struct ct_kernel_task_i0 keyboard_task = {
        .name = task_name,
        .update = update,
        .update_before= update_before,
        .update_after = update_after,
};


void CE_MODULE_LOAD(keyboard)(struct ce_api_a0 *api,
                              int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ct_machine_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);

    _G = (struct G) {
            .ev_queue = ct_machine_a0->new_ev_listener(),
    };

    api->add_api(CT_CONTROLERS_I, &ct_controlers_api, sizeof(ct_controlers_api));
    api->add_impl(CT_KERNEL_TASK_I, &keyboard_task, sizeof(keyboard_task));
}

void CE_MODULE_UNLOAD(keyboard)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _G = (struct G) {};
}