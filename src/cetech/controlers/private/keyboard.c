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

    uint64_t events_n = 0;
    const uint64_t *events = ct_machine_a0->events(&events_n);
    for (int i = 0; i < events_n; ++i) {
        uint64_t event = events[i];
        const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), event);
        uint64_t event_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), event);

        switch (event_type) {
            case EVENT_KEYBOARD_DOWN:
                _G.state[ce_cdb_a0->read_uint64(reader,
                                                CONTROLER_KEYCODE, 0)] = 1;
                break;

            case EVENT_KEYBOARD_UP:
                _G.state[ce_cdb_a0->read_uint64(reader,
                                                CONTROLER_KEYCODE, 0)] = 0;
                break;

            case EVENT_KEYBOARD_TEXT: {
                const char *str = ce_cdb_a0->read_str(reader,
                                                      CONTROLER_TEXT, 0);
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

static struct ct_controlers_i0 ct_controlers_api = {
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

static void _init_api(struct ce_api_a0 *api) {
    api->register_api(CONTROLERS_I, &ct_controlers_api, sizeof(ct_controlers_api));
    api->register_api(KERNEL_TASK_INTERFACE, &keyboard_task, sizeof(keyboard_task));
}

static void _init(struct ce_api_a0 *api) {
    _init_api(api);


    _G = (struct G) {};

    ce_log_a0->debug(LOG_WHERE, "Init");
}

static void _shutdown() {
    ce_log_a0->debug(LOG_WHERE, "Shutdown");

    _G = (struct G) {};
}

void CE_MODULE_LOAD(keyboard)(struct ce_api_a0 *api,
                              int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ct_machine_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);
    _init(api);
}

void CE_MODULE_UNLOAD(keyboard)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _shutdown();
}