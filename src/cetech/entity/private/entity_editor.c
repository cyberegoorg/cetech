#include <stdio.h>
#include <string.h>

#include <celib/memory/allocator.h>
#include <celib/cdb.h>
#include <celib/ydb.h>
#include <celib/math/math.h>

#include <celib/macros.h>
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/module.h"

#include <cetech/ecs/ecs.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/camera/camera.h>
#include <cetech/transform/transform.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/editor/resource_browser.h>
#include <cetech/editor/explorer.h>
#include <cetech/editor/editor.h>
#include <cetech/resource/resource.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/debugui/debugui.h>
#include <cetech/editor/dock.h>
#include <cetech/controlers/controlers.h>
#include <cetech/editor/resource_editor.h>
#include <cetech/editor/selcted_object.h>
#include <celib/containers/array.h>

#define MAX_EDITOR 8

#define _G entity_editor_globals

typedef struct entity_editor {
    ct_world_t0 world;
    struct ct_entity_t0 camera_ent;
    struct ct_entity_t0 entity;
    struct ct_viewport_t0 viewport;

    uint64_t entity_name;
    bool mouse_hovering;
    bool free;
} entity_editor;

static struct _G {
    struct entity_editor *editors;
} _G;


#define _EDITOR_IDX \
    CE_ID64_0("editor_idx", 0x1b333d78a2ccaab7ULL)


static void fps_camera_update(ct_world_t0 world,
                              struct ct_entity_t0 camera_ent,
                              float dt,
                              float dx,
                              float dy,
                              float updown,
                              float leftright,
                              float speed,
                              bool fly_mode) {

//    CE_UNUSED(dx);
//    CE_UNUSED(dy);
//
//    float wm[16];
//
//
//    struct ct_transform_comp *transform;
//    transform = ct_ecs_a0->get_one(world,
//                                                  TRANSFORM_COMPONENT,
//                                                  camera_ent);
//
//    ce_mat4_move(wm, transform->world);
//
//    float x_dir[4];
//    float z_dir[4];
//    ce_vec4_move(x_dir, &wm[0 * 4]);
//    ce_vec4_move(z_dir, &wm[2 * 4]);
//
//    if (!fly_mode) {
//        z_dir[1] = 0.0f;
//    }
//
//    // POS
//    float x_dir_new[3];
//    float z_dir_new[3];
//
//    ce_vec3_mul_s(x_dir_new, x_dir, dt * leftright * speed);
//    ce_vec3_mul_s(z_dir_new, z_dir, dt * updown * speed);
//
//    float pos[3] = {};
//    ce_vec3_add(pos, transform->position, x_dir_new);
//    ce_vec3_add(pos, pos, z_dir_new);

//    uint64_t ent_obj = camera_ent.h;
//    uint64_t components = ce_cdb_a0->read_subobject(ent_obj,
//                                                    ENTITY_COMPONENTS, 0);
//    uint64_t component = ce_cdb_a0->read_subobject(components,
//                                                   TRANSFORM_COMPONENT, 0);
//
//    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(component);
//    ce_cdb_a0->set_vec3(w, PROP_POSITION, pos);
//    ce_cdb_a0->write_commit(w);

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

//static struct ct_component_i0 *get_component_interface(uint64_t cdb_type) {
//    struct ce_api_entry_t0 it = ce_api_a0->first(COMPONENT_INTERFACE);
//    while (it.api) {
//        struct ct_component_i0 *i = (it.api);
//
//        if (cdb_type == i->cdb_type()) {
//            return i;
//        }
//
//        it = ce_api_a0->next(it);
//    }
//
//    return NULL;
//};

//static void _guizmo(uint64_t component_obj,
//                    struct ct_editor_component_i0 *ceditor,
//                    enum OPERATION operation,
//                    const float *view,
//                    const float *proj,
//                    float *size) {
//
//    if (!ceditor->guizmo_get_transform) {
//        return;
//    }
//
//    if (!ceditor->guizmo_set_transform) {
//        return;
//    }
//
//    float world[16];
//    float local[16];
//
//    ce_mat4_identity(world);
//    ce_mat4_identity(local);
//
//    ceditor->guizmo_get_transform(component_obj, world, local);
//
//    float min[2];
//    ct_debugui_a0->GetWindowPos(min);
//    ct_debugui_a0->guizmo_set_rect(min[0], min[1], size[0], size[1]);
//
//    float delta_matrix[16] = {0.0f};
//    ce_mat4_identity(delta_matrix);
//
//    ct_debugui_a0->guizmo_manipulate(view, proj, operation, WORLD,
//                                     world, delta_matrix, 0, NULL, NULL);
//
//    if (!ce_mat4_is_identity(delta_matrix)) {
//        ceditor->guizmo_set_transform(component_obj, operation, world, local);
//    }
//}

static void draw_menu(uint64_t context_obj) {
    static enum OPERATION operation;
    ct_debugui_a0->RadioButton2(ICON_FA_ARROWS_ALT,
                                (int *) &operation, TRANSLATE);

    ct_debugui_a0->SameLine(0, 0);
    ct_debugui_a0->RadioButton2(ICON_FA_UNDO,
                                (int *) &operation, ROTATE);

    ct_debugui_a0->SameLine(0, 0);
    ct_debugui_a0->RadioButton2(ICON_FA_ARROWS_H,
                                (int *) &operation, SCALE);
}

static void draw_editor(uint64_t context_obj,
                        uint64_t context) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), context_obj);

    uint64_t editor_idx = ce_cdb_a0->read_uint64(reader, _EDITOR_IDX, 0);
    struct entity_editor *editor = &_G.editors[editor_idx];

    if (!editor->world.h) {
        return;
    }

    ce_vec2_t size = ct_debugui_a0->GetContentRegionAvail();
    size.y -= ct_debugui_a0->GetTextLineHeightWithSpacing();

    bool is_mouse_hovering = ct_debugui_a0->IsMouseHoveringWindow();
    editor->mouse_hovering = is_mouse_hovering;

    float proj[16], view[16];
    ct_camera_a0->get_project_view(editor->world,
                                   editor->camera_ent,
                                   proj, view,
                                   size.x, size.y);


//    uint64_t obj = 0; //ct_selected_object_a0->selected_object();

//    if (obj) {
//        uint64_t obj_type = ce_cdb_a0->obj_type(obj);
//        if (obj_type == ENTITY_RESOURCE) {
//
//            uint64_t components;
//            components = ce_cdb_a0->read_subobject(obj, ENTITY_COMPONENTS, 0);
//
//            const uint32_t component_n = ce_cdb_a0->prop_count(components);
//
//            const uint64_t *keys = ce_cdb_a0->prop_keys(components);
//
//            for (uint32_t i = 0; i < component_n; ++i) {
//                uint64_t key = keys[i];
//
//                uint64_t component;
//                component = ce_cdb_a0->read_subobject(components, key, 0);
//
//                uint64_t type = ce_cdb_a0->obj_type(component);
//
//                struct ct_component_i0 *c;
//                c = get_component_interface(type);
//
//                if (!c->get_interface) {
//                    continue;
//                }
//
//                struct ct_editor_component_i0 *ceditor;
//                ceditor = c->get_interface(EDITOR_COMPONENT);
//
//                if (!ceditor) {
//                    continue;
//                }
//
//                _guizmo(component, ceditor, operation,
//                        view, proj, size);
//            }
//        }
//    }


    struct ct_rg_builder_t0 *builder = \
    ct_renderer_a0->viewport_builder(editor->viewport);

    builder->set_size(builder, size.x, size.y);

    bgfx_texture_handle_t th;
    th = builder->get_texture(builder, RG_OUTPUT_TEXTURE);

    ct_debugui_a0->Image(th,
                         &size,
                         &(ce_vec4_t) {1.0f, 1.0f, 1.0f, 1.0f},
                         &(ce_vec4_t) {0.0f, 0.0f, 0.0, 0.0f});

}

//static const char *dock_title(ct_dock_i0 *dock) {
//    return ICON_FA_CUBE " Entity editor";
//}
//
//static const char *name(ct_dock_i0 *dock) {
//    return "entity_editor";
//}

static struct entity_editor *_new_editor(uint64_t context_obj) {

    const uint32_t n = ce_array_size(_G.editors);
    for (uint32_t i = 0; i < n; ++i) {
        struct entity_editor *e = &_G.editors[i];

        if (!e->free) {
            continue;
        }

        return e;
    }

    uint32_t idx = n;
    ce_array_push(_G.editors, (entity_editor) {}, ce_memory_a0->system);

    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), context_obj);
    ce_cdb_a0->set_uint64(w, _EDITOR_IDX, idx);
    ce_cdb_a0->write_commit(w);

    struct entity_editor *editor = &_G.editors[idx];

    return editor;
}

static void close(uint64_t context_obj) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), context_obj);
    uint64_t editor_idx = ce_cdb_a0->read_uint64(reader, _EDITOR_IDX, 0);
    struct entity_editor *editor = &_G.editors[editor_idx];

    ct_ecs_a0->destroy_world(editor->world);
    ct_renderer_a0->destroy_viewport(editor->viewport);

    editor->free = true;
}

static void open(uint64_t context_obj) {
    struct entity_editor *editor = _new_editor(context_obj);

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), context_obj);
    const uint64_t asset_name = ce_cdb_a0->read_uint64(reader,
                                                       RESOURCE_EDITOR_OBJ, 0);
    editor->world = ct_ecs_a0->create_world();

    editor->camera_ent = ct_ecs_a0->spawn(editor->world, 0x57899875c4457313);
    editor->viewport = ct_renderer_a0->create_viewport(editor->world,
                                                       editor->camera_ent);

    editor->entity = ct_ecs_a0->spawn(editor->world, asset_name);
}

static void update(uint64_t context_obj,
                   float dt) {
    struct ct_controlers_i0 *keyboard;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), context_obj);
    uint64_t editor_idx = ce_cdb_a0->read_uint64(reader, _EDITOR_IDX, 0);
    struct entity_editor *editor = &_G.editors[editor_idx];

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

uint64_t cdb_type() {
    return ENTITY_RESOURCE_ID;
}

const char *display_icon() {
    return ICON_FA_CUBE;
}

const char *display_name() {
    return "Entity editor";
}


static struct ct_resource_editor_i0 ct_resource_editor_api = {
        .cdb_type = cdb_type,
        .open = open,
        .close = close,
        .update = update,
        .draw_ui = draw_editor,
        .draw_menu = draw_menu,
        .display_name = display_name,
        .display_icon = display_icon,
};

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
    };

    api->register_api(RESOURCE_EDITOR_I, &ct_resource_editor_api, sizeof(ct_resource_editor_api));
}

static void _shutdown() {

    _G = (struct _G) {};
}


void CE_MODULE_LOAD(entity_editor)(struct ce_api_a0 *api,
                                   int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_debugui_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ct_camera_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_rg_a0);
    CE_INIT_API(api, ct_default_rg_a0);
    _init(api);
}

void CE_MODULE_UNLOAD(entity_editor)(struct ce_api_a0 *api,
                                     int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _shutdown();
}
