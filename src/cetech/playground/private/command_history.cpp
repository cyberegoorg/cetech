#include <stdio.h>
#include "cetech/kernel/containers/map.inl"


#include <cetech/engine/debugui/debugui.h>
#include <cetech/playground/playground.h>
#include <cetech/kernel/log/log.h>
#include <cetech/playground/command_history.h>
#include <cetech/playground/command_system.h>
#include <cetech/engine/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/kernel/ebus/ebus.h>

#include "cetech/kernel/hashlib/hashlib.h"
#include "cetech/kernel/memory/memory.h"
#include "cetech/kernel/api/api_system.h"
#include "cetech/kernel/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_playground_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_cmd_system_a0);
CETECH_DECL_API(ct_ebus_a0);

using namespace celib;

#define WINDOW_NAME "Command history"
#define PLAYGROUND_MODULE_NAME CT_ID64_0("command_history")

#define _G command_history_global
static struct _G {
    bool visible;
} _G;


static void ui_command_list() {
    const uint32_t command_count = ct_cmd_system_a0.command_count();
    const uint32_t current_idx = ct_cmd_system_a0.curent_idx();

    char buffer[128];
    char buffer2[128];

    for (uint32_t i = command_count; i > 0; --i) {
        ct_cmd_system_a0.command_text(buffer2, CT_ARRAY_LEN(buffer2), i);
        const bool is_selected = current_idx == i;

        snprintf(buffer, CT_ARRAY_LEN(buffer), "%s##cmd_%d", buffer2, i);

        if (ImGui::Selectable(buffer, is_selected)) {
            ct_cmd_system_a0.goto_idx(i);
            break;
        }
    }
}

static void on_debugui(uint32_t bus_name,
                       void *event) {
    if (ct_debugui_a0.BeginDock(WINDOW_NAME,
                                &_G.visible,
                                DebugUIWindowFlags_(0))) {
        ui_command_list();
    }

    ct_debugui_a0.EndDock();

}

static void on_menu_window(uint32_t bus_name,
                           void *event) {
    ct_debugui_a0.MenuItem2(WINDOW_NAME, NULL, &_G.visible, true);
}


static struct ct_command_history_a0 command_history_api = {
};

static void _init(ct_api_a0 *api) {
    _G = {
            .visible = true,
    };

    api->register_api("ct_command_history_a0", &command_history_api);

    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui, 0);
    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT, on_menu_window, 0);
}

static void _shutdown() {
    ct_ebus_a0.disconnect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui);
    ct_ebus_a0.disconnect(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT, on_menu_window);

    _G = {};
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