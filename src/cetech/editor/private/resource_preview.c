#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/renderer/renderer.h>
#include <cetech/debugui/debugui.h>
#include <cetech/camera/camera.h>
#include <cetech/transform/transform.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/debugdraw/debugdraw.h>

#include <cetech/editor/resource_preview.h>
#include <cetech/editor/resource_browser.h>
#include <cetech/editor/editor.h>
#include <celib/hash.inl>
#include <celib/fmath.inl>

#include <cetech/renderer/gfx.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/editor/dock.h>
#include <cetech/controlers/controlers.h>
#include <cetech/editor/selcted_object.h>
#include <cetech/editor/editor_ui.h>

#include "celib/hashlib.h"
#include "celib/config.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"


#define _G AssetPreviewGlobals

static struct _G {
    struct ce_alloc *allocator;

    uint64_t selected_object;

    struct ct_world world;
    struct ct_entity camera_ent;
    struct ct_viewport0 viewport;

    struct ct_entity active_ent;

    bool visible;

    bool active;
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

//    CE_UNUSED(dx);
//    CE_UNUSED(dy);
//
//    float wm[16];
//
//    struct ct_transform_comp *transform;
//    transform = ct_ecs_a0->get_one(world, TRANSFORM_COMPONENT,
//                                   camera_ent);
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
//
//    float pos[3] = {};
//    ce_vec3_add(transform->position, pos, x_dir_new);
//    ce_vec3_add(pos, pos, z_dir_new);

//
//    uint64_t ent_obj = camera_ent.h;
//    uint64_t components = ce_cdb_a0->read_subobject(ent_obj,
//                                                    ENTITY_COMPONENTS, 0);
//
//    uint64_t component = ce_cdb_a0->read_subobject(components,
//                                                   TRANSFORM_COMPONENT, 0);
//
//    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(component);
//    ce_cdb_a0->set_vec3(w, PROP_POSITION, pos);
//    ce_cdb_a0->write_commit(w);
//
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

static struct ct_resource_preview_i0 *_get_asset_preview(uint64_t asset_type) {
    struct ct_resource_i0 *resource_i;
    resource_i = ct_resource_a0->get_interface(asset_type);

    if (!resource_i) {
        return NULL;
    }

    if (!resource_i->get_interface) {
        return NULL;
    }

    return resource_i->get_interface(RESOURCE_PREVIEW_I);
}

static void set_asset(uint64_t obj) {
    if (_G.selected_object == obj) {
        return;
    }

    if (_G.selected_object) {

        const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                     _G.selected_object);

        uint64_t prev_type = ce_cdb_a0->obj_type(reader);

        struct ct_resource_preview_i0 *i;
        i = _get_asset_preview(prev_type);

        if (i) {
            if (i->unload) {
                i->unload(_G.selected_object, _G.world, _G.active_ent);
            }
        }

    }

    if (obj) {
        const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                     obj);
        uint64_t type = ce_cdb_a0->obj_type(reader);
        struct ct_resource_preview_i0 *i;
        i = _get_asset_preview(type);
        if (i) {
            if (i->load) {
                _G.active_ent = i->load(obj, _G.world);
            }
        }
    }

    _G.selected_object = obj;
}

static void draw_menu(uint64_t dock) {
    ct_dock_a0->context_btn(dock);
    ct_debugui_a0->SameLine(0, -1);
    uint64_t locked_object = ct_editor_ui_a0->lock_selected_obj(dock,
                                                                _G.selected_object);
    if (locked_object) {
        _G.selected_object = locked_object;
    }
}

static void on_debugui(uint64_t dock) {
    _G.active = ct_debugui_a0->IsMouseHoveringWindow();

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), dock);

    const uint64_t context = ce_cdb_a0->read_uint64(reader, PROP_DOCK_CONTEXT,
                                                    0);
    set_asset(ct_selected_object_a0->selected_object(context));

    float size[2];
    ct_debugui_a0->GetContentRegionAvail(size);


    struct ct_rg_builder *builder;
    builder = ct_renderer_a0->viewport_builder(_G.viewport);

    builder->set_size(builder, size[0], size[1]);

    ct_render_texture_handle_t th;
    th = builder->get_texture(builder, RG_OUTPUT_TEXTURE);

    ct_debugui_a0->Image(th,
                         size,
                         (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                         (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
}


static bool init() {
    _G.visible = true;
    _G.world = ct_ecs_a0->create_world();
    _G.camera_ent = ct_ecs_a0->spawn(_G.world, 0x57899875c4457313);
    _G.viewport = ct_renderer_a0->create_viewport(_G.world, _G.camera_ent);

    return true;
}

static void update(float dt) {
    ct_ecs_a0->simulate(_G.world, dt);

    uint64_t selected_object = _G.selected_object;
    if (!selected_object) {
        return;
    }

    struct ct_controlers_i0 *keyboard;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);

    if (_G.active) {
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

        fps_camera_update(_G.world, _G.camera_ent, dt,
                          0, 0, updown, leftright, 10.0f, false);
    }

}

static struct ct_asset_preview_a0 asset_preview_api = {
};

struct ct_asset_preview_a0 *ct_asset_preview_a0 = &asset_preview_api;

static const char *dock_title() {
    return "Asset preview";
}

static const char *name(uint64_t dock) {
    return "asset_preview";
}

static uint64_t cdb_type() {
    return RESOURCE_PREVIEW_I;
};


static struct ct_dock_i0 ct_dock_i0 = {
        .cdb_type = cdb_type,
        .display_title = dock_title,
        .name = name,
        .draw_ui = on_debugui,
        .draw_menu = draw_menu
};


static struct ct_editor_module_i0 ct_editor_module_i0 = {
        .init = init,
        .update = update,
};


static void _init(struct ce_api_a0 *api) {
    api->register_api(DOCK_INTERFACE, &ct_dock_i0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system
    };

    ct_dock_a0->create_dock(RESOURCE_PREVIEW_I, true);

    api->register_api(CT_ASSET_PREVIEW_API, &asset_preview_api);
    api->register_api(EDITOR_MODULE_INTERFACE, &ct_editor_module_i0);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        asset_preview,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ct_camera_a0);
            CE_INIT_API(api, ce_cdb_a0);

            CE_INIT_API(api, ct_dd_a0);
            CE_INIT_API(api, ct_rg_a0);
            CE_INIT_API(api, ct_default_rg_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)