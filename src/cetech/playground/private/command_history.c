#include <stdio.h>

#include <cetech/debugui/debugui.h>
#include <cetech/playground/playground.h>
#include <corelib/log.h>
#include <cetech/playground/command_history.h>
#include <cetech/playground/command_system.h>
#include <corelib/ebus.h>
#include <corelib/macros.h>

#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/module.h"


#define WINDOW_NAME "Command history"
#define PLAYGROUND_MODULE_NAME CT_ID64_0("command_history")

#define _G command_history_global
static struct _G {
    bool visible;
} _G;


static void ui_command_list() {
    const uint32_t command_count = ct_cmd_system_a0->command_count();
    const uint32_t current_idx = ct_cmd_system_a0->curent_idx();

    char buffer[128];
    char buffer2[128];

    for (uint32_t i = command_count; i > 0; --i) {
        ct_cmd_system_a0->command_text(buffer2, CT_ARRAY_LEN(buffer2), i);
        const bool is_selected = current_idx == i;

        snprintf(buffer, CT_ARRAY_LEN(buffer), "%s##cmd_%d", buffer2, i);

        if (ct_debugui_a0->Selectable(buffer, is_selected, 0,
                                      (float[2]) {0.0f})) {
            ct_cmd_system_a0->goto_idx(i);
            break;
        }
    }
}

static void on_debugui(struct ct_dock_i *dock) {
    ui_command_list();
}

static const char *dock_title() {
    return WINDOW_NAME;
}

static struct ct_dock_i ct_dock_i = {
        .id = 0,
        .visible = true,
        .title = dock_title,
        .draw_ui = on_debugui,
};


static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .visible = true,
    };


    api->register_api("ct_dock_i", &ct_dock_i);

}

static void _shutdown() {
    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        command_history,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_playground_a0);
            CETECH_GET_API(api, ct_cmd_system_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_ebus_a0);
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