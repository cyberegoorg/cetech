#include <stdio.h>

#include <cetech/modules/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/renderer/texture.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/level/level.h>
#include <cetech/modules/camera/camera.h>
#include <cetech/modules/playground/level_editor.h>
#include <cetech/modules/transform/transform.h>
#include <cetech/modules/input/input.h>
#include <cetech/modules/renderer/viewport.h>
#include <cetech/modules/playground/asset_browser.h>
#include <cetech/modules/playground/level_inspector.h>
#include <cetech/modules/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/kernel/application.h>
#include <celib/fpumath.h>
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
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_keyboard_a0);
CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_viewport_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_level_inspector_a0);

using namespace celib;

#define MAX_LEVEL_EDITOR 8

static struct globals {
    bool visible[MAX_LEVEL_EDITOR];
    struct ct_viewport viewport[MAX_LEVEL_EDITOR];
    struct ct_world world[MAX_LEVEL_EDITOR];
    struct ct_camera camera[MAX_LEVEL_EDITOR];
    struct ct_entity camera_ent[MAX_LEVEL_EDITOR];
    struct ct_level level[MAX_LEVEL_EDITOR];

    const char* path[MAX_LEVEL_EDITOR];
    uint64_t root[MAX_LEVEL_EDITOR];
    uint64_t level_name[MAX_LEVEL_EDITOR];

    uint8_t active_editor;
    uint8_t editor_count;


} _G;


void fps_camera_update(ct_world world,
                       ct_entity camera_ent,
                       float dt,
                       float dx,
                       float dy,
                       float updown,
                       float leftright,
                       float speed,
                       bool fly_mode) {
    float pos[3];
    float rot[4];
    float wm[16];

    auto transform = ct_transform_a0.get(world, camera_ent);

    ct_transform_a0.get_position(transform, pos);
    ct_transform_a0.get_rotation(transform, rot);
    ct_transform_a0.get_world_matrix(transform, wm);

    float x_dir[4];
    float z_dir[4];
    celib::vec4_move(x_dir, &wm[0 * 4]);
    celib::vec4_move(z_dir, &wm[2 * 4]);


    if (fly_mode) {
        z_dir[1] = 0.0f;
    }

    // POS
    float x_dir_new[3];
    float z_dir_new[3];

    celib::vec3_mul(x_dir_new, x_dir, dt * leftright * speed);
    celib::vec3_mul(z_dir_new, z_dir, dt * updown * speed);

    celib::vec3_add(pos, pos, x_dir_new);
    celib::vec3_add(pos, pos, z_dir_new);

    // ROT
//    float rotation_around_world_up[4];
//    float rotation_around_camera_right[4];
//
//
//
//    local rotation_around_world_up = Quatf.from_axis_angle(Vec3f.unit_y(), -dx * dt * 100)
//    local rotation_around_camera_right = Quatf.from_axis_angle(x_dir, dy * dt * 100)
//    local rotation = rotation_around_world_up * rotation_around_camera_right
//
//    Transform.set_position(self.transform, pos)
//    Transform.set_rotation(self.transform, rot * rotation)
//    end

    ct_transform_a0.set_position(transform, pos);
}

void on_debugui() {
    char dock_id[32] = {};

    _G.active_editor = UINT8_MAX;

    for (uint8_t i = 0; i < _G.editor_count; ++i) {
        snprintf(dock_id, CETECH_ARRAY_LEN(dock_id), "Level view %d", i + 1);

        if (ct_debugui_a0.BeginDock(dock_id, &_G.visible[i],
                                    DebugUIWindowFlags_(
                                            DebugUIWindowFlags_NoScrollbar))) {

            if (ct_debugui_a0.IsMouseHoveringWindow()) {
                _G.active_editor = i;

                if(ImGui::IsMouseClicked(0)) {
                    ct_level_inspector_a0.set_level(_G.world[i], _G.level[i], _G.level_name[i], _G.root[i],
                                                    _G.path[i]);
                }
            }

            auto th = ct_viewport_a0.get_local_resource(
                    _G.viewport[i], ct_hash_a0.id64_from_str("bb_color"));

            float size[2];
            ct_debugui_a0.GetWindowSize(size);
            ct_viewport_a0.resize(_G.viewport[i], size[0], size[1]);
            ct_debugui_a0.Image2(th,
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
        if (!_G.visible[i]) {
            continue;
        }

        ct_viewport_a0.render_world(_G.world[i], _G.camera[i], _G.viewport[i]);
    }
}

void open_level(uint64_t name, uint64_t root, const char* path) {
    uint8_t idx = _G.editor_count;
    ++_G.editor_count;

    _G.visible[idx] = true;

    _G.viewport[idx] = ct_viewport_a0.create(
            ct_hash_a0.id64_from_str("default"), 0, 0);

    _G.world[idx] = ct_world_a0.create();

    _G.camera_ent[idx] = ct_entity_a0.spawn(_G.world[idx],
                                            ct_hash_a0.id64_from_str(
                                                    "camera"));

    _G.camera[idx] = ct_camera_a0.get(_G.world[idx], _G.camera_ent[idx]);

    _G.path[idx] = strdup(path);
    _G.root[idx] = root;
    _G.level_name[idx] = name;

    _G.level[idx] = ct_level_a0.load_level(_G.world[idx], name);

    ct_level_inspector_a0.set_level(_G.world[idx], _G.level[idx], _G.level_name[idx], _G.root[idx], _G.path[idx]);

}

void init() {
}

void shutdown() {
}

void update(float dt) {
    if (UINT8_MAX == _G.active_editor) {
        return;
    }

    float updown = 0.0f;
    float leftright = 0.0f;

    auto up_key = ct_keyboard_a0.button_index("w");
    auto down_key = ct_keyboard_a0.button_index("s");
    auto left_key = ct_keyboard_a0.button_index("a");
    auto right_key = ct_keyboard_a0.button_index("d");

    if (ct_keyboard_a0.button_state(0, up_key) > 0) {
        updown = 1.0f;
    }

    if (ct_keyboard_a0.button_state(0, down_key) > 0) {
        updown = -1.0f;
    }

    if (ct_keyboard_a0.button_state(0, right_key) > 0) {
        leftright = 1.0f;
    }

    if (ct_keyboard_a0.button_state(0, left_key) > 0) {
        leftright = -1.0f;
    }

    fps_camera_update(_G.world[_G.active_editor],
                      _G.camera_ent[_G.active_editor], dt,
                      0, 0,
                      updown, leftright, 10.0f, false);
}

static ct_level_view_a0 level_api = {
//            .register_module = playground::register_module,
//            .unregister_module = playground::unregister_module,
};

static void on_asset_double_click(uint64_t type, uint64_t name, uint64_t root, const char* path) {
    if( ct_hash_a0.id64_from_str("level") != type ) {
        return;
    }

    open_level(name, root, path);
}

static void _init(ct_api_a0 *api) {
    _G = {
            .active_editor = UINT8_MAX
    };

    ct_app_a0.register_on_init(init);
    ct_app_a0.register_on_shutdown(shutdown);
    ct_app_a0.register_on_render(render);
    ct_app_a0.register_on_update(update);
    ct_debugui_a0.register_on_debugui(on_debugui);

    ct_asset_browser_a0.register_on_asset_double_click(on_asset_double_click);

    api->register_api("ct_level_view_a0", &level_api);
}

static void _shutdown() {
    ct_app_a0.unregister_on_init(init);
    ct_app_a0.unregister_on_shutdown(shutdown);
    ct_app_a0.unregister_on_render(render);
    ct_app_a0.unregister_on_update(update);
    ct_debugui_a0.unregister_on_debugui(on_debugui);

    ct_asset_browser_a0.unregister_on_asset_double_click(on_asset_double_click);

    _G = {};
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
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_viewport_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_level_inspector_a0);
        },
        {
            _init(api);
        },
        {
            CEL_UNUSED(api);
            _shutdown();
        }
)