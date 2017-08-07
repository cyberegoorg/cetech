#include <cetech/engine/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/core/blob/blob.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/modules/playground/playground.h>
#include <cetech/engine/application/application.h>
#include "celib/map.inl"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/config/config.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_app_a0);

using namespace celib;

static struct PlaygroundGlobal {
    uint64_t type;

    Map<ct_playground_module> module_map;
} _G;

namespace playground {
    void register_module(ct_playground_module module) {
        uint64_t h = ct_hash_a0.id64_from_str(module.name);
        map::set(_G.module_map, h, module);
    }

    void draw_main_menu() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Quit", "Alt+F4")) {
                    ct_app_a0.quit();
                }
                ImGui::EndMenu();
            }

            //menu_height = ImGui::GetWindowSize().y;

            ImGui::EndMainMenuBar();
        }
    }

    void on_gui() {
        auto ct_it = celib::map::begin(_G.module_map);
        auto ct_end = celib::map::end(_G.module_map);

        bool en = true;

        draw_main_menu();
        while (ct_it != ct_end) {
            auto module = ct_it->value;

            if (ImGui::Begin(module.name, &en)) {
                module.on_gui();
            }
            ImGui::End();

            ++ct_it;
        }

    }
}

namespace playground_module {

    static ct_playground_a0 playground_api = {
            .register_module = playground::register_module
    };

    void on_gui1() {
////    if (ImGui::GetIO().DisplaySize.y > 0) {
////        auto pos = ImVec2(0, 0);
////        auto size = ImGui::GetIO().DisplaySize;
////        size.y -= pos.y;
////        ImGui::RootDock(pos, ImVec2(size.x, size.y));
////    }

        ImGui::Text("dear imgui, %s", ImGui::GetVersion());
        ImGui::Separator();
        ImGui::Text("By Omar Cornut and all github contributors.");
        ImGui::Text(
                "ImGui is licensed under the MIT License, see LICENSE for more information.");
//        if (ImGui::Begin("Asdsladlsa dbout ImGui", &show_app_about2)) {
//            ImGui::Text("dear imgui, %s", ImGui::GetVersion());
//        }
//        ImGui::End();
    }

    void on_gui2() {
        ImGui::Text("dear imgui, %s", ImGui::GetVersion());
        float v[3];
    }

    static void _init(ct_api_a0 *api) {
        api->register_api("ct_playground_a0", &playground_api);

        ct_debugui_a0.register_on_gui(playground::on_gui);

        _G = {};
        _G.module_map.init(ct_memory_a0.main_allocator());

        playground::register_module((ct_playground_module) {
                .name = "window 1 kdsa kdlsa",
                .on_gui = on_gui1,
        });

        playground::register_module((ct_playground_module) {
                .name = "window 2 ddsad sdfdasf",
                .on_gui = on_gui2,
        });
    }

    static void _shutdown() {
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
        },
        {
            playground_module::_init(api);
        },
        {
            CEL_UNUSED(api);
            playground_module::_shutdown();
        }
)