#include <cetech/engine/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/core/blob/blob.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/modules/playground/playground.h>
#include <cetech/engine/application/application.h>
#include <cetech/modules/level/level.h>
#include <cetech/modules/camera/camera.h>
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
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_level_a0);
CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_camera_a0);

using namespace celib;

static struct PlaygroundGlobal {
    uint64_t type;

    Map<ct_playground_module> module_map;

    ct_viewport viewport;
    ct_world world;
    ct_camera camera;
} _G;

namespace playground {
    void register_module(ct_playground_module module) {
        uint64_t h = ct_hash_a0.id64_from_str(module.name);
        map::set(_G.module_map, h, module);

        if(module.on_init) {
            ct_app_a0.register_on_init(module.on_init);
        }

        if(module.on_shutdown) {
            ct_app_a0.register_on_shutdown(module.on_shutdown);
        }

        if(module.on_update) {
            ct_app_a0.register_on_update(module.on_update);
        }

        if(module.on_render) {
            ct_renderer_a0.register_on_render(module.on_render);
        }
    }

    void unregister_module(ct_playground_module module) {
        uint64_t h = ct_hash_a0.id64_from_str(module.name);
        map::remove(_G.module_map, h);

        if(module.on_init) {
            ct_app_a0.unregister_on_init(module.on_init);
        }

        if(module.on_shutdown) {
            ct_app_a0.unregister_on_shutdown(module.on_shutdown);
        }

        if(module.on_update) {
            ct_app_a0.unregister_on_update(module.on_update);
        }

        if(module.on_render) {
            ct_renderer_a0.unregister_on_render(module.on_render);
        }
    }

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
        auto ct_it = celib::map::begin(_G.module_map);
        auto ct_end = celib::map::end(_G.module_map);

        auto menu_height = draw_main_menu();

        uint32_t w, h;
        ct_renderer_a0.get_size(&w, &h);
        auto pos = ImVec2(0, menu_height);

        ImGui::RootDock(pos, ImVec2(w, h - 25.0f));

        bool en = true;
        while (ct_it != ct_end) {
            auto module = ct_it->value;

            if (ImGui::BeginDock(module.name, &en)) {
                module.on_gui();
            }
            ImGui::EndDock();

            ++ct_it;
        }
    }
}

static struct ct_playground_module engine_viewport = {
        .name = "Render",
        .on_shutdown = 0,
        .on_init = []() {
            _G.viewport = ct_renderer_a0.create_viewport(
                    ct_hash_a0.id64_from_str("default"), 0, 0);

            _G.world = ct_world_a0.create();

            auto camera_ent = ct_entity_a0.spawn(_G.world, ct_hash_a0.id64_from_str("camera"));
            _G.camera = ct_camera_a0.get(_G.world, camera_ent);

            ct_level_a0.load_level(_G.world, ct_hash_a0.id64_from_str("level1"));
        },

        .on_gui = []() {
            auto th = ct_renderer_a0.viewport_get_local_resource(
                    _G.viewport,
                    ct_hash_a0.id64_from_str("bb_color"));

            float size[2];
            ct_debugui_a0.GetWindowSize(size);

            ct_debugui_a0.Image2({th},
                                 size,
                                 (float[2]) {0.0f, 0.0f},
                                 (float[2]) {1.0f, 1.0f},
                                 (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                                 (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
        },

        .on_render = [](){
            ct_renderer_a0.render_world(_G.world, _G.camera, _G.viewport);
        }
};

namespace playground_module {

    static ct_playground_a0 playground_api = {
            .register_module = playground::register_module,
            .unregister_module = playground::unregister_module,
    };


    static void _init(ct_api_a0 *api) {
        api->register_api("ct_playground_a0", &playground_api);

        ct_debugui_a0.register_on_gui(playground::on_gui);

        _G = {};
        _G.module_map.init(ct_memory_a0.main_allocator());

        playground::register_module(engine_viewport);
    }

    static void _shutdown() {
        _G.module_map.destroy();

        playground::unregister_module(engine_viewport);

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