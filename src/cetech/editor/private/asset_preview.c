#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/gfx/renderer.h>
#include <cetech/gfx/debugui.h>
#include <cetech/camera/camera.h>
#include <cetech/transform/transform.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/gfx/debugdraw.h>

#include <cetech/editor/asset_preview.h>
#include <cetech/editor/asset_browser.h>
#include <cetech/editor/editor.h>
#include <celib/hash.inl>
#include <celib/fmath.inl>
#include <celib/ebus.h>
#include <cetech/gfx/render_graph.h>
#include <cetech/gfx/default_render_graph.h>
#include <cetech/editor/dock.h>
#include <cetech/controlers/controlers.h>
#include <cetech/resource/sourcedb.h>

#include "celib/hashlib.h"
#include "celib/config.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"


#define _G AssetPreviewGlobals

static struct _G {
    struct ce_alloc *allocator;

    uint64_t selected_object;
    struct ct_resource_id active_asset;

    struct ct_world world;
    struct ct_entity camera_ent;
    struct ct_entity render_ent;

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

    CE_UNUSED(dx);
    CE_UNUSED(dy);

    float wm[16];

    struct ct_transform_comp *transform;
    transform = ct_ecs_a0->component->get_one(world, TRANSFORM_COMPONENT,
                                              camera_ent);

    ce_mat4_move(wm, transform->world);

    float x_dir[4];
    float z_dir[4];
    ce_vec4_move(x_dir, &wm[0 * 4]);
    ce_vec4_move(z_dir, &wm[2 * 4]);

    if (!fly_mode) {
        z_dir[1] = 0.0f;
    }

    // POS
    float x_dir_new[3];
    float z_dir_new[3];

    ce_vec3_mul_s(x_dir_new, x_dir, dt * leftright * speed);
    ce_vec3_mul_s(z_dir_new, z_dir, dt * updown * speed);


    float pos[3] = {};
    ce_vec3_add(transform->position, pos, x_dir_new);
    ce_vec3_add(pos, pos, z_dir_new);

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

static void on_debugui(struct ct_dock_i0 *dock) {
    _G.active = ct_debugui_a0->IsMouseHoveringWindow();


    float size[2];
    ct_debugui_a0->GetContentRegionAvail(size);

    struct ct_render_graph_component *rg_comp;
    rg_comp = ct_ecs_a0->component->get_one(_G.world, RENDER_GRAPH_COMPONENT,
                                            _G.render_ent);

    rg_comp->builder->call->set_size(rg_comp->builder, size[0], size[1]);

    ct_render_texture_handle_t th;
    th = rg_comp->builder->call->get_texture(rg_comp->builder,
                                             RG_OUTPUT_TEXTURE);

    ct_debugui_a0->Image(th,
                         size,
                         (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                         (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
}


static struct ct_asset_preview_i0 *_get_asset_preview(uint64_t asset_type) {
    struct ct_resource_i0 *resource_i;
    resource_i = ct_resource_a0->get_interface(asset_type);

    if (!resource_i) {
        return NULL;
    }

    if (!resource_i->get_interface) {
        return NULL;
    }

    return resource_i->get_interface(ASSET_PREVIEW);
}

static void set_asset(uint64_t type, uint64_t event) {
    uint64_t asset_type = ce_cdb_a0->read_uint64(event, ASSET_TYPE, 0);
    uint64_t asset_name = ce_cdb_a0->read_uint64(event, ASSET_NAME, 0);

    struct ct_resource_id rid = {.name = asset_name, .type = asset_type};

    if (_G.active_asset.name == asset_name &&
        _G.active_asset.type == asset_type) {
        return;
    }

    struct ct_resource_i0 *resource_i;
    resource_i = ct_resource_a0->get_interface(_G.active_asset.type);

    struct ct_asset_preview_i0 *i;
    i = _get_asset_preview(_G.active_asset.type);

    if (i) {
        if (i->unload) {
            i->unload(_G.active_asset, _G.world, _G.active_ent);
        }
    }

    i = _get_asset_preview(rid.type);
    if (i) {
        if (i->load) {
            _G.active_ent = i->load(rid, _G.world);
        }
    }


    _G.active_asset = rid;
}

static bool init() {
    _G.visible = true;
    _G.world = ct_ecs_a0->entity->create_world();
    _G.camera_ent = ct_ecs_a0->entity->spawn(_G.world,
                                             ce_id_a0->id64("content/camera"));

    struct ct_render_graph_component rgc = {
            .builder = ct_render_graph_a0->create_builder(),
            .graph = ct_render_graph_a0->create_graph(),
    };

    ct_ecs_a0->entity->create(_G.world, &_G.render_ent, 1);
    ct_ecs_a0->component->add(_G.world, _G.render_ent,
                              (uint64_t[]) {RENDER_GRAPH_COMPONENT}, 1,
                              (void*[]){&rgc});

    struct ct_render_graph_module *module = ct_default_rg_a0->create(_G.world);
    rgc.graph->call->add_module(rgc.graph, module);

    return true;
}

static void update(float dt) {
    ct_ecs_a0->system->simulate(_G.world, dt);

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

static const char *name(struct ct_dock_i0 *dock) {
    return "asset_preview";
}

static struct ct_dock_i0 ct_dock_i0 = {
        .id = 0,
        .dock_flag = DebugUIWindowFlags_NoNavInputs |
                     DebugUIWindowFlags_NoScrollbar |
                     DebugUIWindowFlags_NoScrollWithMouse,
        .visible = true,
        .display_title = dock_title,
        .name = name,
        .draw_ui = on_debugui,
};


static struct ct_editor_module_i0 ct_editor_module_i0 = {
        .init = init,
        .update = update,
};


static void _on_asset_selected(uint64_t _type, void* event) {
    struct ebus_cdb_event* ev = event;

    uint64_t type = ce_cdb_a0->read_uint64(ev->obj, ASSET_TYPE, 0);
    uint64_t name = ce_cdb_a0->read_uint64(ev->obj, ASSET_NAME, 0);

    struct ct_resource_id rid = {
            .name = name,
            .type = type,
    };

    _G.selected_object = ct_sourcedb_a0->get(rid);

    set_asset(_type, ev->obj);
}

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system
    };

    api->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);
    api->register_api("ct_asset_preview_a0", &asset_preview_api);
    api->register_api("ct_editor_module_i0", &ct_editor_module_i0);

    ce_ebus_a0->connect(ASSET_BROWSER_EBUS,
                        ASSET_BROWSER_ASSET_SELECTED, _on_asset_selected, 0);

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
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ct_dd_a0);
            CE_INIT_API(api, ct_render_graph_a0);
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