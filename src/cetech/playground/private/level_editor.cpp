#include <stdio.h>

#include <cetech/engine/world/world.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/renderer/texture.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/camera/camera.h>
#include <cetech/playground/level_editor.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/engine/input/input.h>
#include <cetech/engine/renderer/viewport.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/playground/explorer.h>
#include <cetech/engine/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/engine/application/application.h>

#include <cetech/playground/playground.h>
#include <cetech/core/math/fmath.h>

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_keyboard_a0);
CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_viewport_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_explorer_a0);
CETECH_DECL_API(ct_playground_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_cdb_a0);

#define MAX_EDITOR 8

static struct globals {
    bool visible[MAX_EDITOR];
    struct ct_viewport viewport[MAX_EDITOR];
    struct ct_world world[MAX_EDITOR];
    struct ct_entity camera_ent[MAX_EDITOR];
    struct ct_entity entity[MAX_EDITOR];
    const char *path[MAX_EDITOR];
    uint64_t root[MAX_EDITOR];
    uint64_t entity_name[MAX_EDITOR];
    bool is_first[MAX_EDITOR];
    bool is_level[MAX_EDITOR];

    uint8_t active_editor;
    uint8_t editor_count;
} _G;


static void fps_camera_update(ct_world world,
                              ct_entity camera_ent,
                              float dt,
                              float dx,
                              float dy,
                              float updown,
                              float leftright,
                              float speed,
                              bool fly_mode) {

    CT_UNUSED(dx);
    CT_UNUSED(dy);

    float pos[3];
    float rot[3];
    float wm[16];

    ct_cdb_obj_t *obj = ct_world_a0.ent_obj(world, camera_ent);

    ct_cdb_a0.read_vec3(obj, PROP_POSITION, pos);
    ct_cdb_a0.read_vec3(obj, PROP_ROTATION, rot);

    ct_transform_a0.get_world_matrix(world, camera_ent, wm);

    float x_dir[4];
    float z_dir[4];
    ct_vec4_move(x_dir, &wm[0 * 4]);
    ct_vec4_move(z_dir, &wm[2 * 4]);

    if (!fly_mode) {
        z_dir[1] = 0.0f;
    }

    // POS
    float x_dir_new[3];
    float z_dir_new[3];

    ct_vec3_mul_s(x_dir_new, x_dir, dt * leftright * speed);
    ct_vec3_mul_s(z_dir_new, z_dir, dt * updown * speed);

    ct_vec3_add(pos, pos, x_dir_new);
    ct_vec3_add(pos, pos, z_dir_new);

    // ROT
//    float rotation_around_world_up[4];
//    float rotation_around_camera_right[4];
//
//    local rotation_around_world_up = Quatf.from_axis_angle(Vec3f.unit_y(), -dx * dt * 100)
//    local rotation_around_camera_right = Quatf.from_axis_angle(x_dir, dy * dt * 100)
//    local rotation = rotation_around_world_up * rotation_around_camera_right
//
//    Transform.set_position(self.transform, pos)
//    Transform.set_rotation(self.transform, rot * rotation)
//    end

    ct_cdb_writer_t *w = ct_cdb_a0.write_begin(obj);
    ct_cdb_a0.set_vec3(w, PROP_POSITION, pos);
    ct_cdb_a0.write_commit(w);
}

static void on_debugui() {
    char dock_id[128] = {};

    _G.active_editor = UINT8_MAX;

    for (uint8_t i = 0; i < _G.editor_count; ++i) {
        if (_G.is_level[i]) {
            snprintf(dock_id, CETECH_ARRAY_LEN(dock_id),
                     "Level %s###level_editor_%d", _G.path[i], i + 1);
        } else {
            snprintf(dock_id, CETECH_ARRAY_LEN(dock_id),
                     "Entity %s###level_editor_%d", _G.path[i], i + 1);
        }

        if (ct_debugui_a0.BeginDock(dock_id,
                                    &_G.visible[i],
                                    DebugUIWindowFlags_(
                                            DebugUIWindowFlags_NoScrollbar))) {

            if (ct_debugui_a0.IsMouseHoveringWindow()) {
                _G.active_editor = i;

                float proj[16], view[16];
                float size[2];
                ct_debugui_a0.GetWindowSize(size);

                ct_camera_a0.get_project_view(_G.world[i], _G.camera_ent[i],
                                              proj, view,
                                              static_cast<int>(size[0]),
                                              static_cast<int>(size[1]));

//                static float im[16] = {
//                        1.0f, 0.0f, 0.0f, 0.0f,
//                        0.0f, 1.0f, 0.0f, 0.0f,
//                        0.0f, 0.0f, 1.0f, 0.0f,
//                        0.0f, 0.0f, 0.0f, 1.0f,
//                };
//                auto origin = ImGui::GetItemRectMin();
//                ImGuizmo::SetRect(origin.x, origin.y, size[0], size[1]);
//                ImGuizmo::Manipulate(view, proj, ImGuizmo::TRANSLATE, ImGuizmo::WORLD, im);

                if (ct_debugui_a0.IsMouseClicked(0, false)) {
                    ct_explorer_a0.set_level(_G.world[i], _G.entity[i],
                                             _G.entity_name[i],
                                             _G.root[i], _G.path[i],
                                             false);
                }
            }

            auto th = ct_viewport_a0.get_local_resource(
                    _G.viewport[i], CT_ID64_0("bb_color"));

            float size[2];
            ct_debugui_a0.GetWindowSize(size);
            ct_viewport_a0.resize(_G.viewport[i], size[0], size[1]);
            ct_debugui_a0.Image2(th,
                                 size,
                                 (float[2]) {0.0f, 0.0f},
                                 (float[2]) {1.0f, 1.0f},
                                 (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                                 (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
        } else {

        }
        ct_debugui_a0.EndDock();
    }
}

static uint32_t find_level(uint64_t name) {
    for (uint32_t i = 0; i < MAX_EDITOR; ++i) {
        if (_G.entity_name[i] != name) {
            continue;
        }

        return i;
    }

    return UINT32_MAX;
}

static void open(uint64_t name,
                 uint64_t root,
                 const char *path,
                 bool is_level) {
    uint32_t level_idx = find_level(name);

    int idx = _G.editor_count;
    ++_G.editor_count;

    _G.visible[idx] = true;
    _G.viewport[idx] = ct_viewport_a0.create(
            CT_ID64_0("default"), 0, 0);

    if (UINT32_MAX != level_idx) {
        _G.world[idx] = _G.world[level_idx];
    } else {
        _G.world[idx] = ct_world_a0.create_world();

        if (is_level) {
            _G.entity[idx] = ct_world_a0.spawn_level(_G.world[idx], name);
        } else {
            _G.entity[idx] = ct_world_a0.spawn_entity(_G.world[idx], name);
        }

        _G.is_first[idx] = true;
        _G.is_level[idx] = is_level;
    }
    _G.camera_ent[idx] = ct_world_a0.spawn_entity(_G.world[idx],
                                                  CT_ID64_0("content/camera"));

    _G.path[idx] = strdup(path);
    _G.root[idx] = root;
    _G.entity_name[idx] = name;

    ct_explorer_a0.set_level(_G.world[idx], _G.entity[idx],
                             _G.entity_name[idx], _G.root[idx],
                             _G.path[idx], is_level);
}

static void init() {
}

static void shutdown() {
}

static void update(float dt) {
    if (UINT8_MAX != _G.active_editor) {

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
                          _G.camera_ent[_G.active_editor],
                          dt, 0, 0, updown, leftright, 10.0f, false);

    }

    for (uint8_t i = 0; i < _G.editor_count; ++i) {
        if (!_G.visible[i]) {
            continue;
        }

        ct_viewport_a0.render_world(_G.world[i], _G.camera_ent[i],
                                    _G.viewport[i]);
    }
}

static ct_level_view_a0 level_api = {
//            .register_module = playground::register_module,
//            .unregister_module = playground::unregister_module,
};

static void on_asset_double_click(uint64_t type,
                                  uint64_t name,
                                  uint64_t root,
                                  const char *path) {
    if (CT_ID64_0("level") == type) {
        open(name, root, path, true);
        return;
    }

    if (CT_ID64_0("entity") == type) {
        open(name, root, path, false);
        return;
    }
}

static void _init(ct_api_a0 *api) {
    _G = {
            .active_editor = UINT8_MAX
    };

    ct_playground_a0.register_module(
            CT_ID64_0("level_editor"),
            (ct_playground_module_fce) {
                    .on_init = init,
                    .on_shutdown = shutdown,
                    .on_update = update,
                    .on_ui = on_debugui,
            });

    ct_asset_browser_a0.register_on_asset_double_click(on_asset_double_click);

    api->register_api("ct_level_view_a0", &level_api);
}

static void _shutdown() {
    ct_playground_a0.unregister_module(CT_ID64_0("level_editor"));
    ct_asset_browser_a0.unregister_on_asset_double_click(on_asset_double_click);

    _G = {};
}

CETECH_MODULE_DEF(
        level_view,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_app_a0);
            CETECH_GET_API(api, ct_world_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_viewport_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_explorer_a0);
            CETECH_GET_API(api, ct_playground_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_cdb_a0);
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