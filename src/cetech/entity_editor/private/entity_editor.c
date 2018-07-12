#include <stdio.h>
#include <string.h>

#include <corelib/cdb.h>
#include <corelib/ydb.h>
#include <corelib/fmath.inl>
#include <corelib/ebus.h>
#include <corelib/macros.h>
#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/module.h"


#include <cetech/ecs/ecs.h>
#include <cetech/renderer/renderer.h>
#include <cetech/debugui/debugui.h>
#include <cetech/camera/camera.h>
#include <cetech/transform/transform.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/asset_browser/asset_browser.h>
#include <cetech/explorer/explorer.h>
#include <cetech/playground/playground.h>
#include <cetech/resource/resource.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_render_graph/default_render_graph.h>
#include <cetech/selected_object/selected_object.h>
#include <cetech/debugui/private/iconfontheaders/icons_font_awesome.h>
#include <cetech/debugui/debugui.h>
#include <cetech/dock/dock.h>
#include <cetech/controlers/controlers.h>
#include <cetech/editor/editor.h>


#define MAX_EDITOR 8

#define _G entity_editor_globals

struct scene_editor {
    struct ct_world world;
    struct ct_entity camera_ent;
    struct ct_entity entity;

    uint32_t entity_name;
    struct ct_render_graph *render_graph;
    struct ct_render_graph_builder *rg_builder;
    bool mouse_hovering;
};

static struct _G {
    struct scene_editor editor[MAX_EDITOR];
    uint8_t editor_count;
} _G;


#define _EDITOR_IDX \
    CT_ID64_0("editor_idx", 0x1b333d78a2ccaab7ULL)


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
    transform = ct_ecs_a0->component->entity_data(world,
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

    uint64_t ent_obj = ct_ecs_a0->entity->cdb_object(world, camera_ent);
    uint64_t components = ct_cdb_a0->read_subobject(ent_obj,
                                                    ENTITY_COMPONENTS, 0);
    uint64_t component = ct_cdb_a0->read_subobject(components,
                                                   TRANSFORM_COMPONENT, 0);

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(component);
    ct_cdb_a0->set_vec3(w, PROP_POSITION, pos);
    ct_cdb_a0->write_commit(w);

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

static struct ct_component_i0 *get_component_interface(uint64_t cdb_type) {
    struct ct_api_entry it = ct_api_a0->first("ct_component_i0");
    while (it.api) {
        struct ct_component_i0 *i = (it.api);

        if (cdb_type == i->cdb_type()) {
            return i;
        }

        it = ct_api_a0->next(it);
    }

    return NULL;
};

static void _guizmo(uint64_t component_obj,
                    struct ct_editor_component_i0 *ceditor,
                    enum OPERATION operation,
                    const float *view,
                    const float *proj,
                    float *size) {

    if (!ceditor->guizmo_get_transform) {
        return;
    }

    if (!ceditor->guizmo_set_transform) {
        return;
    }

    float world[16];
    float local[16];

    ct_mat4_identity(world);
    ct_mat4_identity(local);

    ceditor->guizmo_get_transform(component_obj, world, local);

    float min[2];
    ct_debugui_a0->GetWindowPos(min);
    ct_debugui_a0->guizmo_set_rect(min[0], min[1], size[0], size[1]);

    float delta_matrix[16] = {0.0f};
    ct_mat4_identity(delta_matrix);

    ct_debugui_a0->guizmo_manipulate(view, proj, operation, WORLD,
                                     world, delta_matrix, 0, NULL, NULL);

    if (!ct_mat4_is_identity(delta_matrix)) {
        ceditor->guizmo_set_transform(component_obj, operation, world, local);
    }
}

static void draw_editor(uint64_t context_obj) {
    uint64_t editor_idx = ct_cdb_a0->read_uint64(context_obj, _EDITOR_IDX, 0);
    struct scene_editor *editor = &_G.editor[editor_idx];

    if (!editor->world.h) {
        return;
    }

    float size[2];
    ct_debugui_a0->GetContentRegionAvail(size);
    size[1] -= ct_debugui_a0->GetTextLineHeightWithSpacing();

    bool is_mouse_hovering = ct_debugui_a0->IsMouseHoveringWindow();
    editor->mouse_hovering = is_mouse_hovering;

    static enum OPERATION operation;
    ct_debugui_a0->RadioButton2(ICON_FA_ARROWS_ALT,
                                (int *) &operation, TRANSLATE);

    ct_debugui_a0->SameLine(0, 0);
    ct_debugui_a0->RadioButton2(ICON_FA_UNDO,
                                (int *) &operation, ROTATE);

    ct_debugui_a0->SameLine(0, 0);
    ct_debugui_a0->RadioButton2(ICON_FA_ARROWS_H,
                                (int *) &operation, SCALE);

    float proj[16], view[16];
    ct_camera_a0->get_project_view(editor->world,
                                   editor->camera_ent,
                                   proj, view,
                                   size[0], size[1]);


    uint64_t obj = ct_selected_object_a0->selected_object();

    if (obj) {
        uint64_t obj_type = ct_cdb_a0->type(obj);
        if (obj_type == ENTITY_RESOURCE) {

            uint64_t components;
            components = ct_cdb_a0->read_subobject(obj, ENTITY_COMPONENTS, 0);

            const uint32_t component_n = ct_cdb_a0->prop_count(components);

            uint64_t keys[component_n];
            ct_cdb_a0->prop_keys(components, keys);

            for (uint32_t i = 0; i < component_n; ++i) {
                uint64_t key = keys[i];

                uint64_t component;
                component = ct_cdb_a0->read_subobject(components, key, 0);

                uint64_t type = ct_cdb_a0->type(component);

                struct ct_component_i0 *c;
                c = get_component_interface(type);

                if (!c->get_interface) {
                    continue;
                }

                struct ct_editor_component_i0 *ceditor;
                ceditor = c->get_interface(EDITOR_COMPONENT);

                if (!ceditor) {
                    continue;
                }

                _guizmo(component, ceditor, operation,
                        view, proj, size);


            }
        }
    }

    if (ct_debugui_a0->IsMouseClicked(0, false)) {
//        ct_selected_object_a0->set_selected_object(editor->asset);
//        ct_explorer_a0->set_level(editor->world,
//                                  editor->entity,
//                                  editor->entity_name,
//                                  editor->root,
//                                  editor->path);
    }

    ct_render_texture_handle_t th;
    th = editor->rg_builder->call->get_texture(editor->rg_builder,
                                               RG_OUTPUT_TEXTURE);

    editor->rg_builder->call->set_size(editor->rg_builder,
                                       size[0], size[1]);


    ct_debugui_a0->Image(th,
                         size,
                         (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                         (float[4]) {0.0f, 0.0f, 0.0, 0.0f});

}

//static const char *dock_title(struct ct_dock_i0 *dock) {
//    return ICON_FA_CUBE " Entity editor";
//}
//
//static const char *name(struct ct_dock_i0 *dock) {
//    return "entity_editor";
//}

static struct scene_editor *_new_editor(uint64_t context_obj) {
    int idx = _G.editor_count;
    ++_G.editor_count;


    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(context_obj);
    ct_cdb_a0->set_uint64(w, _EDITOR_IDX, idx);
    ct_cdb_a0->write_commit(w);

    struct scene_editor *editor = &_G.editor[idx];

    return editor;
}

static void open(uint64_t context_obj) {
    struct scene_editor *editor = _new_editor(context_obj);


    const uint64_t asset_name = ct_cdb_a0->read_uint64(context_obj, _ASSET_NAME,
                                                       0);

    editor->world = ct_ecs_a0->entity->create_world();
    editor->entity = ct_ecs_a0->entity->spawn(editor->world, asset_name);

    editor->render_graph = ct_render_graph_a0->create_graph();
    editor->rg_builder = ct_render_graph_a0->create_builder();
    editor->render_graph->call->add_module(editor->render_graph,
                                           ct_default_rg_a0->create(
                                                   editor->world));


    editor->camera_ent = ct_ecs_a0->entity->spawn(editor->world,
                                                  CT_ID32_0("content/camera"));

    editor->entity_name = asset_name;

//    ct_selected_object_a0->set_selected_object(obj);
//    ct_explorer_a0->set_level(editor->world,
//                              editor->entity,
//                              editor->entity_name,
//                              editor->root,
//                              editor->path);

}

static void update(uint64_t context_obj,
                   float dt) {
    struct ct_controlers_i0 *keyboard;
    keyboard = ct_controlers_a0->get_by_name(CONTROLER_KEYBOARD);

    uint64_t editor_idx = ct_cdb_a0->read_uint64(context_obj, _EDITOR_IDX, 0);
    struct scene_editor *editor = &_G.editor[editor_idx];

    if (!editor->world.h) {
        return;
    }

    if (editor->mouse_hovering) {
        float updown = 0.0f;
        float leftright = 0.0f;

        uint32_t up_key = keyboard->button_index("w");
        uint32_t down_key = keyboard->button_index("s");
        uint32_t left_key = keyboard->button_index("a");
        uint32_t right_key = keyboard->button_index("d");

        if (keyboard->button_state(0, up_key) > 0) {
            updown = 1.0f;
        }

        if (keyboard->button_state(0, down_key) > 0) {
            updown = -1.0f;
        }

        if (keyboard->button_state(0, right_key) > 0) {
            leftright = 1.0f;
        }

        if (keyboard->button_state(0, left_key) > 0) {
            leftright = -1.0f;
        }

        fps_camera_update(editor->world,
                          editor->camera_ent,
                          dt, 0, 0, updown, leftright, 10.0f, false);
    }

    ct_ecs_a0->simulate(editor->world, dt);

}

static void on_render(uint64_t context_obj) {
    uint64_t editor_idx = ct_cdb_a0->read_uint64(context_obj, _EDITOR_IDX, 0);
    struct scene_editor *editor = &_G.editor[editor_idx];
    if (!editor->world.h) {
        return;
    }

    editor->rg_builder->call->clear(editor->rg_builder);


    editor->render_graph->call->setup(editor->render_graph,
                                      editor->rg_builder);

    editor->rg_builder->call->execute(editor->rg_builder);

}

uint64_t asset_type() {
    return ENTITY_RESOURCE_ID;
}

static struct ct_asset_editor_i0 ct_asset_editor_i0 = {
        .asset_type = asset_type,
        .open = open,
        .update = update,
        .render = on_render,
        .draw = draw_editor,
};

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
    };

    api->register_api("ct_asset_editor_i0", &ct_asset_editor_i0);
}

static void _shutdown() {

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        entity_editor,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_render_graph_a0);
            CETECH_GET_API(api, ct_default_rg_a0);
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
