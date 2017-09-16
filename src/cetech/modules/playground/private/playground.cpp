#include <stdio.h>

#include <celib/macros.h>
#include <celib/map.inl>

#include <cetech/kernel/filesystem.h>
#include <cetech/kernel/vio.h>
#include <cetech/kernel/ydb.h>
#include "cetech/kernel/hashlib.h"
#include "cetech/kernel/memory.h"
#include "cetech/kernel/api_system.h"
#include "cetech/kernel/module.h"

#include <cetech/modules/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/playground/playground.h>
#include <cetech/modules/application/application.h>
#include <cetech/modules/level/level.h>
#include <cetech/modules/camera/camera.h>
#include <cetech/modules/renderer/viewport.h>
#include <cetech/modules/playground/command_system.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_level_a0);
CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_cmd_system_a0);

using namespace celib;

static struct PlaygroundGlobal {
    uint64_t type;

    ct_viewport viewport;
    ct_world world;
    ct_camera camera;

    bool load_layout;

    Map<ct_playground_module_fce> module_map;
} _G;

namespace playground {
    float draw_main_menu() {
        float menu_height = 0;
        static bool debug = false;

        if (ct_debugui_a0.BeginMainMenuBar()) {
            if (ct_debugui_a0.BeginMenu("File", true)) {
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
                const char* txt = ct_cmd_system_a0.undo_text();

                sprintf(buffer, "Undo %s", txt ? txt : "");
                if (ct_debugui_a0.MenuItem(buffer, "Ctrl+Z", false, NULL!=txt)) {
                    ct_cmd_system_a0.undo();
                }

                txt = ct_cmd_system_a0.redo_text();
                sprintf(buffer, "Redo %s", txt ? txt : "");
                if (ct_debugui_a0.MenuItem(buffer, "Ctrl+Shift+Z", false, NULL!=txt)) {
                    ct_cmd_system_a0.redo();
                }

                ct_debugui_a0.EndMenu();
            }

            if (ct_debugui_a0.BeginMenu("Layout", true)) {
                if (ct_debugui_a0.MenuItem("Save", NULL, false, true )) {
                    ct_vio *f = ct_filesystem_a0.open(ct_hash_a0.id64_from_str("source"), "core/default.dock_layout", FS_OPEN_WRITE);
                    ct_debugui_a0.SaveDock(f);
                    ct_filesystem_a0.close(f);
                }

                if (ct_debugui_a0.MenuItem("Load", NULL, false, true)) {
                    ct_debugui_a0.LoadDock("core/default.dock_layout");
                }
                ct_debugui_a0.EndMenu();
            }

            if (ct_debugui_a0.BeginMenu("Run", true)) {
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

    void on_debugui() {
        auto menu_height = draw_main_menu();

        uint32_t w, h;
        ct_renderer_a0.get_size(&w, &h);
        float pos[] = {0.0f, menu_height};
        float size[] = {float(w), h - 25.0f};

        ct_debugui_a0.RootDock(pos, size);
    }
}

static void on_init(){

    auto *it = map::begin(_G.module_map);
    auto *it_end = map::end(_G.module_map);

    while (it != it_end) {
        if(it->value.on_init){
            it->value.on_init();
        }

        ++it;
    }
}

static void on_shutdown() {
    auto *it = map::begin(_G.module_map);
    auto *it_end = map::end(_G.module_map);

    while (it != it_end) {
        if(it->value.on_shutdown) {
            it->value.on_shutdown();
        }
        ++it;
    }
}

void reload_layout() {
    _G.load_layout = true;
}

static void on_update(float dt) {
    auto *it = map::begin(_G.module_map);
    auto *it_end = map::end(_G.module_map);

    while (it != it_end) {
        if(it->value.on_update) {
            it->value.on_update(dt);
        }
        ++it;
    }
}

static bool active_hack = false;
static void on_render() {
    active_hack = true;

    auto *it = map::begin(_G.module_map);
    auto *it_end = map::end(_G.module_map);
    while (it != it_end) {
        if(it->value.on_render) {
            it->value.on_render();
        }

        ++it;
    }
}


static void on_ui() {
    if(!active_hack) {
        return;
    }

    active_hack = false;
    playground::on_debugui();

    auto *it = map::begin(_G.module_map);
    auto *it_end = map::end(_G.module_map);

    while (it != it_end) {
        if(it->value.on_ui) {
            it->value.on_ui();
        }
        ++it;
    }

    if(_G.load_layout) {
        ct_debugui_a0.LoadDock("core/default.dock_layout");
        _G.load_layout = false;
    }
}

static ct_game_fce playground_game {
        .on_init = on_init,
        .on_shutdown = on_shutdown,
        .on_update   = on_update,
        .on_render =  on_render,
        .on_ui =  on_ui,
};


void register_module(uint64_t name, ct_playground_module_fce module) {
    celib::map::set(_G.module_map, name, module);
}

void unregister_module(uint64_t name) {
    celib::map::remove(_G.module_map, name);
}

static ct_playground_a0 playground_api = {
        .register_module = register_module,
        .unregister_module = unregister_module,
        .reload_layout = reload_layout,
};

namespace playground_module {
    static void _init(ct_api_a0 *api) {
        _G = {
             .load_layout = true,
        };

        _G.module_map.init(ct_memory_a0.main_allocator());

        api->register_api("ct_playground_a0", &playground_api);

        ct_app_a0.register_game(ct_hash_a0.id64_from_str("playground"), playground_game);
        ct_debugui_a0.register_on_debugui(on_ui);
    }

    static void _shutdown() {
        ct_debugui_a0.unregister_on_debugui(on_ui);
        ct_app_a0.unregister_game(ct_hash_a0.id64_from_str("playground"));
        _G.module_map.destroy();

        _G = {};
    }
}

CETECH_MODULE_DEF(
        playground,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_app_a0);
            CETECH_GET_API(api, ct_world_a0);
            CETECH_GET_API(api, ct_level_a0);
            CETECH_GET_API(api, ct_entity_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_cmd_system_a0);
        },
        {
            CEL_UNUSED(reload);
            playground_module::_init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            playground_module::_shutdown();
        }
)