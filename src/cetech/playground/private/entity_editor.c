#include <stdio.h>

#include <cetech/kernel/cdb/cdb.h>
#include <cetech/kernel/yaml/ydb.h>
#include <cetech/engine/ecs/ecs.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/debugui/debugui.h>

#include <cetech/engine/camera/camera.h>
#include <cetech/playground/entity_editor.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/engine/controlers/keyboard.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/playground/explorer.h>

#include <cetech/playground/playground.h>
#include <cetech/kernel/math/fmath.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/kernel/ebus/ebus.h>
#include <cetech/engine/render_graph/render_graph.h>
#include <cetech/engine/default_render_graph/default_render_graph.h>
#include <cetech/kernel/macros.h>
#include <string.h>

#include "cetech/kernel/hashlib/hashlib.h"
#include "cetech/kernel/memory/memory.h"
#include "cetech/kernel/api/api_system.h"
#include "cetech/kernel/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_keyboard_a0);
CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_explorer_a0);
CETECH_DECL_API(ct_playground_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_ebus_a0);
CETECH_DECL_API(ct_render_graph_a0);
CETECH_DECL_API(ct_default_render_graph_a0);

#define MAX_EDITOR 8

#define _G entity_editor_globals

static struct _G {
    bool visible[MAX_EDITOR];
    struct ct_world world[MAX_EDITOR];
    struct ct_entity camera_ent[MAX_EDITOR];
    struct ct_entity entity[MAX_EDITOR];
    const char *path[MAX_EDITOR];
    uint64_t root[MAX_EDITOR];
    uint32_t entity_name[MAX_EDITOR];
    bool is_first[MAX_EDITOR];
    struct ct_render_graph *render_graph[MAX_EDITOR];
    struct ct_render_graph_builder *render_graph_builder[MAX_EDITOR];

    uint8_t active_editor;
    uint8_t editor_count;
} _G;


static void fps_camera_update(struct ct_world world,
                              struct ct_entity camera_ent,
                              float dt,
                              float dx,
                              float dy,
                              float updown,
                              float leftright,
                              float speed,
                              bool fly_mode) {

    CT_UNUSED(dx);
    CT_UNUSED(dy);

    float wm[16];


    struct ct_transform_comp *transform;
    transform = ct_ecs_a0.entity_data(
            world,
            TRANSFORM_COMPONENT,
            camera_ent);

    ct_mat4_move(wm, transform->world);

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

    float pos[3] = {0};
    ct_vec3_add(pos, transform->position, x_dir_new);
    ct_vec3_add(pos, pos, z_dir_new);

    ct_cdb_obj_o *w = ct_cdb_a0.write_begin(ct_ecs_a0.entity_object(world, camera_ent));
    ct_cdb_a0.set_vec3(w, PROP_POSITION, pos);
    ct_cdb_a0.write_commit(w);

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
}

static void on_debugui(uint64_t event) {
    char dock_id[128] = {};

    _G.active_editor = UINT8_MAX;

    for (uint8_t i = 0; i < _G.editor_count; ++i) {
        snprintf(dock_id, CT_ARRAY_LEN(dock_id),
                 "Entity %s###level_editor_%d", _G.path[i], i + 1);

        if (ct_debugui_a0.BeginDock(dock_id, &_G.visible[i],
                                    DebugUIWindowFlags_NoScrollbar)) {

            if (ct_debugui_a0.IsMouseHoveringWindow()) {
                _G.active_editor = i;

//                float proj[16], view[16];
//                float size[2];
//                ct_debugui_a0.GetWindowSize(size);
//
//                ct_camera_a0.get_project_view(_G.world[i], _G.camera_ent[i],
//                                              proj, view,
//                                              static_cast<int>(size[0]),
//                                              static_cast<int>(size[1]));
//
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
                                             _G.root[i], _G.path[i]);
                }
            }

//            uint64_t obj = ct_ecs_a0.ent_obj(_G.world[i],
//                                                           _G.camera_ent[i]);
//

            ct_render_texture_handle_t th;
            th = _G.render_graph_builder[i]->call->get_texture(
                    _G.render_graph_builder[i], CT_ID64_0("output"));

            float size[2];
            ct_debugui_a0.GetWindowSize(size);

            _G.render_graph_builder[i]->call->set_size(
                    _G.render_graph_builder[i], size[0], size[1]);

//            ct_viewport_a0.resize(camera_data->viewport, size[0], size[1]);
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

static uint32_t find_entity(uint32_t name) {
    for (uint32_t i = 0; i < MAX_EDITOR; ++i) {
        if (_G.entity_name[i] != name) {
            continue;
        }

        return i;
    }

    return UINT32_MAX;
}

static void open(struct ct_resource_id asset,
                 uint64_t root,
                 const char *path) {

    uint32_t ent_idx = find_entity(asset.name);

    int idx = _G.editor_count;
    ++_G.editor_count;

    _G.visible[idx] = true;

    if (UINT32_MAX != ent_idx) {
        _G.world[idx] = _G.world[ent_idx];
    } else {
        _G.world[idx] = ct_ecs_a0.create_world();

        _G.entity[idx] = ct_ecs_a0.spawn_entity(_G.world[idx], asset.name);

        _G.is_first[idx] = true;

        _G.render_graph[idx] = ct_render_graph_a0.create_graph();
        _G.render_graph_builder[idx] = ct_render_graph_a0.create_builder();
        _G.render_graph[idx]->call->add_module(_G.render_graph[idx],
                                               ct_default_render_graph_a0.create(
                                                       _G.world[idx]));
    }
    _G.camera_ent[idx] = ct_ecs_a0.spawn_entity(_G.world[idx],
                                                CT_ID32_0("content/camera"));

    _G.path[idx] = strdup(path);
    _G.root[idx] = root;
    _G.entity_name[idx] = asset.name;

    ct_explorer_a0.set_level(_G.world[idx], _G.entity[idx],
                             _G.entity_name[idx], _G.root[idx],
                             _G.path[idx]);
}

static void update(uint64_t event) {
    float dt = ct_cdb_a0.read_float(event, CT_ID64_0("dt"), 0.0f);

    if (UINT8_MAX != _G.active_editor) {

        float updown = 0.0f;
        float leftright = 0.0f;

        uint32_t up_key = ct_keyboard_a0.button_index("w");
        uint32_t down_key = ct_keyboard_a0.button_index("s");
        uint32_t left_key = ct_keyboard_a0.button_index("a");
        uint32_t right_key = ct_keyboard_a0.button_index("d");

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

        ct_ecs_a0.simulate(_G.world[i], dt);
    }
}

static void on_render(uint64_t event) {
    for (uint8_t i = 0; i < _G.editor_count; ++i) {
        _G.render_graph_builder[i]->call->clear(_G.render_graph_builder[i]);

        if (!_G.visible[i]) {
            continue;
        }

        _G.render_graph[i]->call->setup(_G.render_graph[i],
                                        _G.render_graph_builder[i]);
        _G.render_graph_builder[i]->call->execute(_G.render_graph_builder[i]);
    }
}

static struct ct_entity_editor_a0 level_api = {
//            .register_module = playground::register_module,
//            .unregister_module = playground::unregister_module,
};

static void on_asset_double_click(uint64_t event) {
    uint64_t asset = ct_cdb_a0.read_uint64(event, CT_ID64_0("asset"), 0);
    uint64_t root = ct_cdb_a0.read_uint64(event, CT_ID64_0("root"), 0);
    const char* path = ct_cdb_a0.read_str(event, CT_ID64_0("path"), 0);

    struct ct_resource_id rid = {.i64 = asset};

    if (CT_ID32_0("entity") == rid.type) {
        open(rid, root, path);
        return;
    }
}

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G){
            .active_editor = UINT8_MAX
    };


    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UPDATE_EVENT, update, 0);
    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui, 0);
    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_RENDER_EVENT, on_render, 0);
    ct_ebus_a0.connect(ASSET_BROWSER_EBUS, ASSET_DCLICK_EVENT,
                       on_asset_double_click, 0);

    api->register_api("ct_level_view_a0", &level_api);
}

static void _shutdown() {
    ct_ebus_a0.disconnect(PLAYGROUND_EBUS, PLAYGROUND_UPDATE_EVENT, update);
    ct_ebus_a0.disconnect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui);
    ct_ebus_a0.disconnect(PLAYGROUND_EBUS, PLAYGROUND_RENDER_EVENT, on_render);
    ct_ebus_a0.disconnect(ASSET_BROWSER_EBUS, ASSET_DCLICK_EVENT,
                          on_asset_double_click);

    _G = (struct _G){};
}

CETECH_MODULE_DEF(
        entity_editor,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_explorer_a0);
            CETECH_GET_API(api, ct_playground_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_render_graph_a0);
            CETECH_GET_API(api, ct_default_render_graph_a0);
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