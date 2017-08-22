#include <cetech/entity/entity.h>
#include <cetech/renderer/renderer.h>
#include <cetech/debugui/debugui.h>
#include <cetech/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/playground/playground.h>
#include <cetech/application/application.h>
#include <cetech/level/level.h>
#include <cetech/camera/camera.h>
#include <cetech/renderer/viewport.h>
#include "celib/map.inl"

#include "cetech/hashlib/hashlib.h"
#include "cetech/config/config.h"
#include "cetech/memory/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_level_a0);
CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_camera_a0);

using namespace celib;

static struct PlaygroundGlobal {
    uint64_t type;

    ct_viewport viewport;
    ct_world world;
    ct_camera camera;
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

    void on_gui() {
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
        api->register_api("ct_playground_a0", &playground_api);

        ct_debugui_a0.register_on_gui(playground::on_gui);

        _G = {};
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
        },
        {
            playground_module::_init(api);
        },
        {
            CEL_UNUSED(api);
            playground_module::_shutdown();
        }
)