#include <cetech/modules/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/playground/playground.h>
#include <cetech/kernel/application.h>
#include <cetech/modules/level/level.h>
#include <cetech/modules/camera/camera.h>
#include <cetech/modules/renderer/viewport.h>
#include <cetech/kernel/filesystem.h>
#include <cetech/kernel/vio.h>
#include <celib/macros.h>

#include "cetech/kernel/hashlib.h"
#include "cetech/kernel/memory.h"
#include "cetech/kernel/api_system.h"
#include "cetech/kernel/module.h"

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

static struct PlaygroundGlobal {
    uint64_t type;

    ct_viewport viewport;
    ct_world world;
    ct_camera camera;

    bool layout_loaded;
} _G;

namespace playground {
    float draw_main_menu() {
        float menu_height = 0;
        static bool debug = false;

        if (ct_debugui_a0.BeginMainMenuBar()) {
            if (ct_debugui_a0.BeginMenu("File", true)) {
                if (ct_debugui_a0.MenuItem("Quit", "Alt+F4", false, true)) {
                    ct_app_a0.quit();
                }

                if (ct_debugui_a0.MenuItem("Settings", NULL, false, true)) {
                }

                if (ct_debugui_a0.MenuItem2("Debug", "F9", &debug, true)) {
                    ct_renderer_a0.set_debug(debug);
                }

                ct_debugui_a0.EndMenu();
            }

            if (ct_debugui_a0.BeginMenu("Edit", true)) {
                ct_debugui_a0.EndMenu();
            }

            if (ct_debugui_a0.BeginMenu("Layout", true)) {
                if (ct_debugui_a0.MenuItem("Save", NULL, false, true )) {
                    ct_vio *f = ct_filesystem_a0.open(ct_hash_a0.id64_from_str("source"), "default.dock_layout", FS_OPEN_WRITE);
                    ct_debugui_a0.SaveDock(f);
                    ct_filesystem_a0.close(f);
                }

                if (ct_debugui_a0.MenuItem("Load", NULL, false, true)) {
                    ct_debugui_a0.LoadDock("default.dock_layout");
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

namespace playground_module {

    static ct_playground_a0 playground_api = {
//            .register_module = playground::register_module,
//            .unregister_module = playground::unregister_module,
    };


    static void _init(ct_api_a0 *api) {
        _G = {};

        api->register_api("ct_playground_a0", &playground_api);

        ct_debugui_a0.register_on_debugui(playground::on_debugui);

        if(!_G.layout_loaded) {
            ct_debugui_a0.LoadDock("default.dock_layout");
            _G.layout_loaded = true;
        }
    }

    static void _shutdown() {
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