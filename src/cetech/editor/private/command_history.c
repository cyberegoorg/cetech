#include <stdio.h>

#include <cetech/debugui/debugui.h>
#include <cetech/editor/editor.h>
#include <celib/log.h>
#include <cetech/command_system/command_system.h>
#include <celib/ebus.h>
#include <celib/macros.h>

#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"
#include <cetech/editor/dock.h>
#include <string.h>

#define WINDOW_NAME "Command history"

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
        ct_cmd_system_a0->command_text(buffer2, CE_ARRAY_LEN(buffer2), i);
        const bool is_selected = current_idx == i;

        snprintf(buffer, CE_ARRAY_LEN(buffer), "%s##cmd_%d", buffer2, i);

        if (ct_debugui_a0->Selectable(buffer, is_selected, 0,
                                      (float[2]) {0.0f})) {
            ct_cmd_system_a0->goto_idx(i);
            break;
        }
    }
}

static void on_debugui(uint64_t dock) {
    ui_command_list();
}

static const char *dock_title() {
    return WINDOW_NAME;
}

static const char *name(uint64_t dock) {
    return "command_history";
}

static struct ct_dock_i0 ct_dock_i0 = {
        .display_title = dock_title,
        .name = name,
        .draw_ui = on_debugui,
};


static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .visible = true,
    };


    api->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        command_history,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ct_cmd_system_a0);
            CE_INIT_API(api, ce_log_a0);
            CE_INIT_API(api, ce_ebus_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)