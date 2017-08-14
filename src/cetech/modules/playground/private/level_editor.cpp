#include <cetech/engine/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/engine/application/application.h>
#include <cetech/modules/level/level.h>
#include <cetech/modules/camera/camera.h>
#include <cetech/modules/playground/level_editor.h>
#include <cstdio>
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

#define MAX_LEVEL_EDITOR 8

static struct {
    bool visible[MAX_LEVEL_EDITOR];
    ct_viewport viewport[MAX_LEVEL_EDITOR];
    ct_world world[MAX_LEVEL_EDITOR];
    ct_camera camera[MAX_LEVEL_EDITOR];
    ct_entity camera_ent[MAX_LEVEL_EDITOR];

    uint8_t editor_count;
} _G;

namespace level_view {
    void gui() {
//    fps_camera_update(_G.world, _G.camera_ent, 16 * 0.00001f, 0, 0, 1.0f, 0.0f,
//                      0);

        char dock_id[32] = {};

        for (uint8_t i = 0; i < _G.editor_count; ++i) {
            snprintf(dock_id, CETECH_ARRAY_LEN(dock_id), "Level view #%d", i+1);

            if (ct_debugui_a0.BeginDock(dock_id, &_G.visible[i],
                                        DebugUIWindowFlags_Empty)) {
                auto th = ct_renderer_a0.viewport_get_local_resource(
                        _G.viewport[i], ct_hash_a0.id64_from_str("bb_color"));

                float size[2];
                ct_debugui_a0.GetWindowSize(size);
                ct_renderer_a0.resize_viewport(_G.viewport[i], size[0], size[1]);
                ct_debugui_a0.Image2({th},
                                     size,
                                     (float[2]) {0.0f, 0.0f},
                                     (float[2]) {1.0f, 1.0f},
                                     (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                                     (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
            }
            ct_debugui_a0.EndDock();
        }
    }

    void render() {
        for (uint8_t i = 0; i < _G.editor_count; ++i) {
            ct_renderer_a0.render_world(_G.world[i], _G.camera[i],
                                        _G.viewport[i]);
        }
    }

    void open_level() {
        uint8_t idx = _G.editor_count;
        ++_G.editor_count;

        _G.visible[idx] = true;

        _G.viewport[idx] = ct_renderer_a0.create_viewport(
                ct_hash_a0.id64_from_str("default"), 0, 0);

        _G.world[idx] = ct_world_a0.create();

        _G.camera_ent[idx] = ct_entity_a0.spawn(_G.world[idx],
                                                ct_hash_a0.id64_from_str(
                                                        "camera"));

        _G.camera[idx] = ct_camera_a0.get(_G.world[idx], _G.camera_ent[idx]);

        ct_level_a0.load_level(_G.world[idx],
                               ct_hash_a0.id64_from_str("level1"));
    }

    void init() {
        open_level();
        open_level();
    }

    void shutdown() {

    }
}

namespace level_view_module {

    static ct_level_view_a0 api = {
//            .register_module = playground::register_module,
//            .unregister_module = playground::unregister_module,
    };

    static void _init(ct_api_a0 *api) {
        api->register_api("ct_level_view_a0", &api);

        _G = {};

        ct_app_a0.register_on_init(level_view::init);
        ct_app_a0.register_on_shutdown(level_view::shutdown);
        ct_app_a0.register_on_render(level_view::render);
        ct_debugui_a0.register_on_gui(level_view::gui);
    }

    static void _shutdown() {
        ct_app_a0.unregister_on_init(level_view::init);
        ct_app_a0.unregister_on_shutdown(level_view::shutdown);
        ct_app_a0.unregister_on_render(level_view::render);
        ct_debugui_a0.unregister_on_gui(level_view::gui);

        _G = {};
    }
}

CETECH_MODULE_DEF(
        level_view,
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
            level_view_module::_init(api);
        },
        {
            CEL_UNUSED(api);
            level_view_module::_shutdown();
        }
)