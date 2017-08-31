#include <cetech/modules/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/modules/playground/playground.h>
#include <cetech/kernel/application.h>
#include <cetech/modules/level/level.h>
#include <cetech/modules/camera/camera.h>
#include <cetech/modules/renderer/viewport.h>
#include <cetech/kernel/filesystem.h>
#include <cetech/kernel/vio.h>
#include "celib/map.inl"

#include "cetech/kernel/hashlib.h"
#include "cetech/kernel/config.h"
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

using namespace celib;

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
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Quit", "Alt+F4")) {
                    ct_app_a0.quit();
                }

                if (ImGui::MenuItem("Settings")) {
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Layout")) {
                if (ImGui::MenuItem("Save")) {
                    ct_vio *f = ct_filesystem_a0.open(ct_hash_a0.id64_from_str("source"), "default.dock_layout", FS_OPEN_WRITE);
                    ct_debugui_a0.SaveDock(f);
                    ct_filesystem_a0.close(f);
                }

                if (ImGui::MenuItem("Load")) {
                    ct_debugui_a0.LoadDock("default.dock_layout");
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Run")) {
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                }
                ImGui::EndMenu();
            }

            menu_height = ImGui::GetWindowSize().y;

            ImGui::EndMainMenuBar();
        }
        return menu_height;
    }

    void on_debugui() {
        auto menu_height = draw_main_menu();

        uint32_t w, h;
        ct_renderer_a0.get_size(&w, &h);
        auto pos = ImVec2(0, menu_height);

        ImGui::RootDock(pos, ImVec2(w, h - 25.0f));
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
            playground_module::_init(api);
        },
        {
            CEL_UNUSED(api);
            playground_module::_shutdown();
        }
)