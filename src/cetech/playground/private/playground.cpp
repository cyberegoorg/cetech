#include <stdio.h>

#include <cetech/core/macros.h>
#include <cetech/core/containers/map.inl>

#include <cetech/core/fs/fs.h>
#include <cetech/core/os/vio.h>
#include <cetech/core/yaml/ydb.h>
#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/module/module.h"

#include <cetech/engine/ecs/ecs.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/playground/playground.h>
#include <cetech/engine/application/application.h>
#include <cetech/engine/renderer/viewport.h>
#include <cetech/engine/camera/camera.h>
#include <cetech/engine/renderer/viewport.h>
#include <cetech/playground/command_system.h>
#include <cetech/engine/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/playground/action_manager.h>
#include <cetech/core/ebus/ebus.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_fs_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_cmd_system_a0);
CETECH_DECL_API(ct_action_manager_a0);
CETECH_DECL_API(ct_module_a0);
CETECH_DECL_API(ct_ebus_a0);

using namespace celib;

static struct PlaygroundGlobal {
    bool load_layout;
} _G;

static float draw_main_menu() {
    float menu_height = 0;
    static bool debug = false;

    if (ct_debugui_a0.BeginMainMenuBar()) {
        if (ct_debugui_a0.BeginMenu("File", true)) {
            if (ct_debugui_a0.MenuItem("Reload", "Alt+r", false, true)) {
                ct_module_a0.reload_all();
            }

            if (ct_debugui_a0.MenuItem("Save", "Alt+s", false, true)) {
                ct_ydb_a0.save_all_modified();
            }

            if (ct_debugui_a0.MenuItem2("Debug", "F9", &debug, true)) {
                ct_renderer_a0.set_debug(debug);
            }

            if (ct_debugui_a0.MenuItem("Quit", "Alt+F4", false, true)) {
                ct_app_a0.quit();
            }

            ct_debugui_a0.EndMenu();
        }

        if (ct_debugui_a0.BeginMenu("Edit", true)) {
            char buffer[128];
            char buffer2[128];

            ct_cmd_system_a0.undo_text(buffer2, CT_ARRAY_LEN(buffer2));
            const char *shortcut;

            sprintf(buffer, "Undo %s", buffer2[0] != '0' ? buffer2 : "");

            shortcut = ct_action_manager_a0.shortcut_str(CT_ID64_0("undo"));
            if (ct_debugui_a0.MenuItem(buffer, shortcut, false,
                                       buffer2[0] != '0')) {
                ct_action_manager_a0.execute(CT_ID64_0("undo"));
            }


            ct_cmd_system_a0.redo_text(buffer2, CT_ARRAY_LEN(buffer2));
            shortcut = ct_action_manager_a0.shortcut_str(CT_ID64_0("redo"));
            sprintf(buffer, "Redo %s", buffer2[0] != '0' ? buffer2 : "");
            if (ct_debugui_a0.MenuItem(buffer, shortcut, false,
                                       buffer2[0] != '0')) {
                ct_action_manager_a0.execute(CT_ID64_0("redo"));
            }

            ct_debugui_a0.EndMenu();
        }

        if (ct_debugui_a0.BeginMenu("Window", true)) {
            if (ct_debugui_a0.BeginMenu("Layout", true)) {
                if (ct_debugui_a0.MenuItem("Save", NULL, false, true)) {
                    ct_vio *f = ct_fs_a0.open(CT_ID64_0("source"),
                                                      "core/default.dock_layout",
                                                      FS_OPEN_WRITE);
                    ct_debugui_a0.SaveDock(f);
                    ct_fs_a0.close(f);
                }

                if (ct_debugui_a0.MenuItem("Load", NULL, false, true)) {
                    ct_debugui_a0.LoadDock("core/default.dock_layout");
                }
                ct_debugui_a0.EndMenu();
            }

            ImGui::Separator();

            ct_ebus_a0.broadcast(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT, NULL,
                            0);

            ct_debugui_a0.EndMenu();
        }

        if (ct_debugui_a0.BeginMenu("Help", true)) {
            if (ct_debugui_a0.MenuItem("About", NULL, false, true)) {
            }
            ct_debugui_a0.EndMenu();
        }

        float v[2];
        ct_debugui_a0.GetWindowSize(v);
        menu_height = v[1];

        ct_debugui_a0.EndMainMenuBar();
    }
    return menu_height;
}

static void on_debugui() {
    auto menu_height = draw_main_menu();

    uint32_t w, h;
    ct_renderer_a0.get_size(&w, &h);
    float pos[] = {0.0f, menu_height};
    float size[] = {float(w), h - 25.0f};

    ct_debugui_a0.RootDock(pos, size);
}

static void on_init() {
    ct_ebus_a0.broadcast(PLAYGROUND_EBUS, PLAYGROUND_INIT_EVENT, NULL, 0);
}

static void on_shutdown() {
    ct_ebus_a0.broadcast(PLAYGROUND_EBUS, PLAYGROUND_SHUTDOWN_EVENT, NULL, 0);

}

void reload_layout() {
    _G.load_layout = true;
}

static void on_update(float dt) {
    ct_action_manager_a0.check();

    ct_playground_update_ev ev = {.dt=dt};
    ct_ebus_a0.broadcast(PLAYGROUND_EBUS, PLAYGROUND_UPDATE_EVENT, &ev, sizeof(ev));
}



static void on_ui() {
    on_debugui();

    ct_ebus_a0.broadcast(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, NULL, 0);

    if (_G.load_layout) {
        ct_debugui_a0.LoadDock("core/default.dock_layout");
        _G.load_layout = false;
    }
}

static ct_game_fce playground_game{
        .on_init = on_init,
        .on_shutdown = on_shutdown,
        .on_update   = on_update,
        .on_ui =  on_ui,
};


static ct_playground_a0 playground_api = {
        .reload_layout = reload_layout,
};

static void _init(ct_api_a0 *api) {
    _G = {
            .load_layout = true,
    };

    api->register_api("ct_playground_a0", &playground_api);

    ct_ebus_a0.create_ebus(PLAYGROUND_EBUS_NAME, PLAYGROUND_EBUS);

    ct_action_manager_a0.register_action(
            CT_ID64_0("undo"),
            "ctrl+z",
            ct_cmd_system_a0.undo
    );

    ct_action_manager_a0.register_action(
            CT_ID64_0("redo"),
            "ctrl+shift+z",
            ct_cmd_system_a0.redo
    );

    ct_app_a0.register_game(CT_ID64_0("playground"), playground_game);
    ct_debugui_a0.register_on_debugui(on_ui);
}

static void _shutdown() {
    ct_debugui_a0.unregister_on_debugui(on_ui);
    ct_app_a0.unregister_game(CT_ID64_0("playground"));

    _G = {};
}

CETECH_MODULE_DEF(
        playground,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_app_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_fs_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_action_manager_a0);
            CETECH_GET_API(api, ct_cmd_system_a0);
            CETECH_GET_API(api, ct_module_a0);
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