#include <celib/memory/allocator.h>
#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/camera/camera.h>
#include <cetech/transform/transform.h>

#include <cetech/resource_preview/resource_preview.h>
#include <cetech/editor/editor.h>
#include <celib/containers/hash.h>
#include <celib/math/math.h>

#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/editor/dock.h>
#include <cetech/resource/resourcedb.h>

#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/module.h"


#define _G AssetPreviewGlobals

CE_MODULE(ct_resourcedb_a0);

typedef struct preview_instance {
    ct_world_t0 world;
    ct_entity_t0 camera_ent;
    ct_entity_t0 ent;
    uint64_t selected_object;
    uint64_t type;
//    bool locked;
    bool free;
} preview_instance;

static struct _G {
    ce_alloc_t0 *allocator;

    preview_instance *instances;
    preview_instance *baground;

    bool visible;
    bool active;
} _G;

static struct preview_instance *_new_preview() {
    const uint32_t n = ce_array_size(_G.instances);
    for (uint32_t i = 0; i < n; ++i) {
        struct preview_instance *pi = &_G.instances[i];

        if (!pi->free) {
            continue;
        }

        return pi;
    }

    uint32_t idx = n;
    ce_array_push(_G.instances, ((preview_instance) {}), ce_memory_a0->system);
    preview_instance *pi = &_G.instances[idx];

    return pi;
}

static struct ct_resource_preview_i0 *_get_asset_preview(uint64_t asset_type) {
    ct_resource_i0 *resource_i;
    resource_i = ct_resource_a0->get_interface(asset_type);

    if (!resource_i) {
        return NULL;
    }

    if (!resource_i->get_interface) {
        return NULL;
    }

    return resource_i->get_interface(RESOURCE_PREVIEW_I);
}

static void set_asset(preview_instance *pi,
                      uint64_t obj) {
    if (!pi) {
        return;
    }

//    if (pi->locked) {
//        return;
//    }

    obj = ce_cdb_a0->find_root(ce_cdb_a0->db(), obj);

    if (pi->selected_object == obj) {
        return;
    }

    if (pi->selected_object && pi->ent.h) {

        uint64_t prev_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(),
                                                 pi->selected_object);

        struct ct_resource_preview_i0 *i;
        i = _get_asset_preview(prev_type);

        if (i) {
            if (i->unload) {
                i->unload(pi->selected_object, pi->world, pi->ent);
            }
        }

        ct_ecs_e_a0->destroy_entities(pi->world, &pi->ent, 1);
        //ct_ecs_a0->step(pi->world, 0);
        pi->ent.h = 0;
        pi->type = 0;
    }

    if (obj) {
        uint64_t type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), obj);

        pi->type = type;

        struct ct_resource_preview_i0 *i;
        i = _get_asset_preview(type);
        if (i) {
            if (i->load) {
                pi->ent = i->load(obj, pi->world);
            }
        }
    }

    pi->selected_object = obj;
}

static void _draw_preview(preview_instance *pi,
                          ce_vec2_t size) {

    if (!pi->type) {
        return;
    }

    ct_resource_preview_i0 *i;
    i = _get_asset_preview(pi->type);

    if (!i) {
        return;
    }

    if (i->draw_raw) {
        i->draw_raw(pi->selected_object, size);
    } else {
        viewport_component *viewport = ct_ecs_c_a0->get_one(pi->world, VIEWPORT_COMPONENT,
                                                            pi->camera_ent, false);

        struct ct_rg_builder_t0 *builder;
        builder = ct_renderer_a0->viewport_builder(viewport->viewport);

        ct_renderer_a0->viewport_set_size(viewport->viewport, size);

        bgfx_texture_handle_t th;
        th = builder->get_texture(builder, RG_OUTPUT_TEXTURE);

        ct_debugui_a0->Image(th,
                             &size,
                             &(ce_vec4_t) {1.0f, 1.0f, 1.0f, 1.0f},
                             &(ce_vec4_t) {0.0f, 0.0f, 0.0f, 0.0f});
    }
}

static void draw_dock(uint64_t content,
                      uint64_t context,
                      uint64_t selected_object) {
    _G.active = ct_debugui_a0->IsMouseHoveringWindow();

    preview_instance *pi = (preview_instance *) content;

    if (!pi) {
        return;
    }

    set_asset(pi, selected_object);

    ce_vec2_t size = ct_debugui_a0->GetContentRegionAvail();
    _draw_preview(pi, size);
}

static bool init() {
    _G.visible = true;

    preview_instance *pi = _new_preview();

    _G.baground = pi;

    pi->world = ct_ecs_a0->create_world("background resource preview");

    ct_ecs_e_a0->create_entities(pi->world, &pi->camera_ent, 1);

    ct_ecs_c_a0->add(pi->world,
                     pi->camera_ent,
                     CE_ARR_ARG(((ct_component_pair_t0[]) {
                             {
                                     .type = POSITION_COMPONENT,
                                     .data = &(ct_position_c) {
                                     }
                             },
                             {
                                     .type = LOCAL_TO_WORLD_COMPONENT,
                                     .data = &(ct_local_to_world_c) {
                                     }
                             },
                             {
                                     .type = CT_CAMERA_COMPONENT,
                                     .data = &(ct_camera_component) {
                                             .camera_type = CAMERA_TYPE_PERSPECTIVE,
                                             .near = 0.1f,
                                             .far = 1000.0f,
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

    ct_dock_a0->create_dock(RESOURCE_PREVIEW_I, true);
    return true;
}

static void update(float dt) {
    uint32_t n = ce_array_size(_G.instances);
    for (int i = 0; i < n; ++i) {
        struct preview_instance *pi = &_G.instances[i];

        if (ct_ecs_c_a0->has(pi->world, pi->ent, (uint64_t[]) {ROTATION_COMPONENT}, 1)) {
            ct_rotation_c *rot_t = ct_ecs_c_a0->get_one(pi->world,
                                                        ROTATION_COMPONENT, pi->ent, true);

            ce_vec4_t q = ce_quat_from_euler(0, 1 * CE_DEG_TO_RAD, 0);
            rot_t->rot = ce_quat_mul(q, rot_t->rot);
        }

        ct_ecs_a0->step(pi->world, dt);
    }
}

void set_background_resource(ct_resource_id_t0 resource) {
    set_asset(_G.baground, resource.uid);
}

void draw_background_texture(ce_vec2_t size) {
    _draw_preview(_G.baground, size);
}


static void resource_tooltip(ct_resource_id_t0 resourceid,
                             const char *path,
                             ce_vec2_t size) {
    ct_debugui_a0->Text("%s", path);

    uint64_t type = ct_resourcedb_a0->get_resource_type(resourceid);

    ct_resource_i0 *ri = ct_resource_a0->get_interface(type);

    if (!ri || !ri->get_interface) {
        return;
    }

    ct_resource_preview_i0 *ai = (ri->get_interface(RESOURCE_PREVIEW_I));

    uint64_t obj = resourceid.uid;

    if (ai) {
        if (ai->tooltip) {
            ai->tooltip(obj, size);
        }

        set_background_resource(resourceid);
        draw_background_texture(size);
    }
}

static struct ct_resource_preview_a0 asset_preview_api = {
        .resource_tooltip = resource_tooltip,
};

struct ct_resource_preview_a0 *ct_resource_preview_a0 = &asset_preview_api;

static const char *dock_title() {
    return "Resource preview";
}

static const char *name(uint64_t dock) {
    return "asset_preview";
}

static uint64_t open(uint64_t dock) {
    preview_instance *pi = _new_preview();

    pi->world = ct_ecs_a0->create_world("resource preview");

    ct_ecs_e_a0->create_entities(pi->world, &pi->camera_ent, 1);
    ct_ecs_c_a0->add(pi->world,
                     pi->camera_ent,
                     CE_ARR_ARG(((ct_component_pair_t0[]) {
                             {
                                     .type = POSITION_COMPONENT,
                                     .data = &(ct_position_c) {
                                     }
                             },
                             {
                                     .type = LOCAL_TO_WORLD_COMPONENT,
                                     .data = &(ct_local_to_world_c) {
                                     }
                             },
                             {
                                     .type = CT_CAMERA_COMPONENT,
                                     .data = &(ct_camera_component) {
                                             .camera_type = CAMERA_TYPE_PERSPECTIVE,
                                             .far = 100.0f,
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

    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), dock);
    ce_cdb_a0->set_ptr(w, PROP_DOCK_DATA, pi);
    ce_cdb_a0->write_commit(w);

    return (uint64_t) pi;
}


static struct ct_dock_i0 dock_api = {
        .type = RESOURCE_PREVIEW_I,
        .display_title = dock_title,
        .name = name,
        .draw_ui = draw_dock,
        .open = open,
};


static struct ct_editor_module_i0 ct_editor_module_api = {
        .init = init,
        .update = update,
};


void CE_MODULE_LOAD(asset_preview)(struct ce_api_a0 *api,
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
            .allocator = ce_memory_a0->system
    };

    api->add_api(CT_ASSET_PREVIEW_API, &asset_preview_api, sizeof(asset_preview_api));
    api->add_impl(CT_DOCK_I, &dock_api, sizeof(dock_api));
    api->add_impl(CT_EDITOR_MODULE_I, &ct_editor_module_api, sizeof(ct_editor_module_api));
}

void CE_MODULE_UNLOAD(asset_preview)(struct ce_api_a0 *api,
                                     int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
