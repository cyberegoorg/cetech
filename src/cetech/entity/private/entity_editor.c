#include <string.h>

#include <celib/memory/allocator.h>
#include <celib/cdb.h>
#include <celib/yaml_cdb.h>

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
#include <cetech/resource/resource.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/resource_editor/resource_editor.h>
#include <celib/containers/array.h>
#include <cetech/controlers/controlers.h>
#include <cetech/controlers/keyboard.h>
#include <celib/math/math.h>
#include <cetech/controlers/mouse.h>
#include <celib/log.h>
#include <float.h>

#define _G entity_editor_globals

typedef struct entity_editor {
    ct_world_t0 world;
    ct_entity_t0 camera_ent;
    ct_entity_t0 entity;

    bool mouse_hovering;
    bool free;
} entity_editor;

static struct _G {
    entity_editor *editors;
} _G;

static void draw_menu(uint64_t context_obj) {
    entity_editor *editor = (entity_editor *) context_obj;

    static enum OPERATION operation;
    ct_debugui_a0->RadioButton2(ICON_FA_ARROWS_ALT, (int *) &operation, TRANSLATE);

    ct_debugui_a0->SameLine(0, 0);
    ct_debugui_a0->RadioButton2(ICON_FA_UNDO, (int *) &operation, ROTATE);

    ct_debugui_a0->SameLine(0, 0);
    ct_debugui_a0->RadioButton2(ICON_FA_ARROWS_H, (int *) &operation, SCALE);

    ct_debugui_a0->SameLine(0, -1);

    ct_debugui_a0->Text("%s", "C:");
    ct_debugui_a0->SameLine(0, -1);

    ct_camera_component *camera = ct_ecs_c_a0->get_one(editor->world,
                                                       CT_CAMERA_COMPONENT, editor->camera_ent,
                                                       true);
    int cur_item = 0;

    if (camera->camera_type == CAMERA_TYPE_ORTHO) {
        cur_item = 1;
    }

    if (ct_debugui_a0->Combo("", &cur_item,
                             (const char *[]) {"perspective", "ortho"}, 2, -1)) {
        if (cur_item == 0) {
            camera->camera_type = CAMERA_TYPE_PERSPECTIVE;
        } else if (cur_item == 1) {
            camera->camera_type = CAMERA_TYPE_ORTHO;
        }
    };
}

static void draw_editor(uint64_t context_obj,
                        uint64_t context) {
    entity_editor *editor = (entity_editor *) context_obj;

    if (!editor->world.h) {
        return;
    }

    ce_vec2_t size = ct_debugui_a0->GetContentRegionAvail();
//    size.y -= ct_debugui_a0->GetTextLineHeightWithSpacing();

    bool is_mouse_hovering = ct_debugui_a0->IsMouseHoveringWindow();
    editor->mouse_hovering = is_mouse_hovering;

    viewport_component *viewport = ct_ecs_c_a0->get_one(editor->world, VIEWPORT_COMPONENT,
                                                        editor->camera_ent, false);

    ct_rg_builder_t0 *builder = ct_renderer_a0->viewport_builder(viewport->viewport);

    ct_renderer_a0->viewport_set_size(viewport->viewport, size);

    bgfx_texture_handle_t th;
    th = builder->get_texture(builder, RG_OUTPUT_TEXTURE);

    ct_debugui_a0->Image(th,
                         &size,
                         &(ce_vec4_t) {1.0f, 1.0f, 1.0f, 1.0f},
                         &(ce_vec4_t) {0.0f, 0.0f, 0.0, 0.0f});
}

static struct entity_editor *_new_editor() {

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

    entity_editor *editor = &_G.editors[idx];

    editor->world = ct_ecs_a0->create_world("editor");

    ct_ecs_e_a0->create_entities(editor->world, &editor->camera_ent, 1);
    ct_ecs_c_a0->add(editor->world,
                     editor->camera_ent,
                     CE_ARR_ARG(((ct_component_pair_t0[]) {
                             {
                                     .type = POSITION_COMPONENT,
                                     .data = &(ct_position_c) {
                                     }
                             },
                             {
                                     .type = ROTATION_COMPONENT,
                                     .data = &(ct_rotation_c) {
                                             .rot = CE_QUAT_IDENTITY
                                     }
                             },

                             {
                                     .type = LOCAL_TO_WORLD_COMPONENT,
                                     .data = &(ct_local_to_world_c) {
                                             .world = CE_MAT4_IDENTITY,
                                     }
                             },
                             {
                                     .type = CT_CAMERA_COMPONENT,
                                     .data = &(ct_camera_component) {
                                             .camera_type = CAMERA_TYPE_PERSPECTIVE,
                                             .far = 10000.0f,
                                             .near = 0.1f,
                                             .fov = 60.0f,
                                     }
                             },
                             {
                                     .type = VIEWPORT_COMPONENT,
                                     .data = &(viewport_component) {
                                             .viewport =ct_renderer_a0->create_viewport(),
                                     }
                             }
                     })));

    return editor;
}

static void close(uint64_t context_obj) {
    entity_editor *editor = (entity_editor *) context_obj;

    ct_ecs_e_a0->destroy_entities(editor->world, &editor->entity, 1);
    editor->free = true;
}

static uint64_t open(uint64_t obj) {
    entity_editor *editor = _new_editor();
    editor->entity = ct_ecs_e_a0->spawn_entity(editor->world, obj);
    return (uint64_t) editor;
}

static void update(uint64_t context_obj,
                   float dt) {
    entity_editor *editor = (entity_editor *) context_obj;

    if (!editor->world.h) {
        return;
    }

    if (editor->mouse_hovering) {
        ct_position_c *pos = ct_ecs_c_a0->get_one(editor->world,
                                                  POSITION_COMPONENT,
                                                  editor->camera_ent, true);

        ct_rotation_c *rot = ct_ecs_c_a0->get_one(editor->world,
                                                  ROTATION_COMPONENT,
                                                  editor->camera_ent, true);

        ct_controler_i0 *kb = ct_controlers_a0->get(CONTROLER_KEYBOARD);

        float forward = kb->button_state(0, kb->button_index("w")) ? 1.0f :
                        kb->button_state(0, kb->button_index("s")) ? -1.0f : 0.0f;

        float side = kb->button_state(0, kb->button_index("d")) ? 1.0f :
                     kb->button_state(0, kb->button_index("a")) ? -1.0f : 0.0f;

        ce_vec3_t fw_dir = ce_vec3_mul_quat(CE_VEC3_UNIT_Z, rot->rot);
        fw_dir = ce_vec3_mul_s(fw_dir, forward * 1.0f);

        ce_vec3_t rg_dir = ce_vec3_mul_quat(CE_VEC3_UNIT_X, rot->rot);
        rg_dir = ce_vec3_mul_s(rg_dir, side * 1.0f);

        pos->pos = ce_vec3_add(pos->pos, ce_vec3_add(fw_dir, rg_dir));

        ct_controler_i0 *mouse = ct_controlers_a0->get(CONTROLER_MOUSE);
        if (mouse->button_state(0, mouse->button_index("left"))) {
            ce_vec3_t axis = {};
            mouse->axis(0, mouse->axis_index("relative"), &axis.x);

            if (!ce_vec3_equal(axis, CE_VEC3_ZERO, FLT_EPSILON)) {
//                ce_vec4_t q_x = ce_quat_rotate_x(axis.y * CE_DEG_TO_RAD);
                ce_vec4_t q_y = ce_quat_rotate_y(-(axis.x * CE_DEG_TO_RAD));

//                rot->rot = ce_quat_mul(q_x, rot->rot);
                rot->rot = ce_quat_mul(q_y, rot->rot);
            }
        }
    }

    ct_ecs_a0->step(editor->world, dt);
}

uint64_t cdb_type() {
    return ENTITY_TYPE;
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

    _G = (struct _G) {
    };

    api->add_impl(CT_RESOURCE_EDITOR_I, &ct_resource_editor_api, sizeof(ct_resource_editor_api));
}

void CE_MODULE_UNLOAD(entity_editor)(struct ce_api_a0 *api,
                                     int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _G = (struct _G) {};
}
