#include <celib/memory/allocator.h>
#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/camera/camera.h>
#include <cetech/transform/transform.h>
#include <cetech/controlers/keyboard.h>

#include <cetech/resource/resource_preview.h>
#include <cetech/resource/resource_browser.h>
#include <cetech/editor/editor.h>
#include <celib/containers/hash.h>
#include <celib/math/math.h>

#include <cetech/renderer/gfx.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/editor/dock.h>
#include <cetech/controlers/controlers.h>
#include <cetech/editor/selcted_object.h>
#include <cetech/editor/editor_ui.h>

#include "celib/id.h"
#include "celib/config.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/module.h"


#define _G AssetPreviewGlobals

#define PREVIEW_PTR \
    CE_ID64_0("preview_ptr", 0x1e2c71526a2e8a11ULL)

typedef struct preview_instance {
    ct_world_t0 world;
    ct_entity_t0 camera_ent;
    ct_viewport_t0 viewport;
    ct_entity_t0 ent;
    uint64_t selected_object;
    uint64_t type;
    bool locked;
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
    ce_array_push(_G.instances,
                  ((preview_instance) {}),
                  ce_memory_a0->system);

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

    if (pi->locked) {
        return;
    }

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

        ct_ecs_a0->destroy(pi->world, &pi->ent, 1);

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

static void draw_menu(uint64_t dock) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), dock);
    preview_instance *pi = ce_cdb_a0->read_ptr(reader,
                                               PREVIEW_PTR, NULL);

    ct_dock_a0->context_btn(dock);
    ct_debugui_a0->SameLine(0, -1);
    uint64_t locked_object = ct_editor_ui_a0->lock_selected_obj(dock, pi->selected_object);

    pi->locked = false;

    if (locked_object) {
        pi->selected_object = locked_object;
        pi->locked = true;
    }
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

        struct ct_rg_builder_t0 *builder;
        builder = ct_renderer_a0->viewport_builder(pi->viewport);

        ct_renderer_a0->viewport_set_size(pi->viewport, size);

        bgfx_texture_handle_t th;
        th = builder->get_texture(builder, RG_OUTPUT_TEXTURE);

        ct_debugui_a0->Image(th,
                             &size,
                             &(ce_vec4_t) {1.0f, 1.0f, 1.0f, 1.0f},
                             &(ce_vec4_t) {0.0f, 0.0f, 0.0, 0.0f});
    }
}

static void draw_dock(uint64_t dock) {
    _G.active = ct_debugui_a0->IsMouseHoveringWindow();

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), dock);
    preview_instance *pi = ce_cdb_a0->read_ptr(reader, PREVIEW_PTR, NULL);

    if (!pi) {
        return;
    }

    const uint64_t context = ce_cdb_a0->read_uint64(reader, PROP_DOCK_CONTEXT, 0);
    set_asset(pi, ct_selected_object_a0->selected_object(context));

    ce_vec2_t size = ct_debugui_a0->GetContentRegionAvail();
    _draw_preview(pi, size);
}

static bool init() {
    _G.visible = true;

    preview_instance *pi = _new_preview();

    _G.baground = pi;

    pi->world = ct_ecs_a0->create_world();
    pi->camera_ent = ct_ecs_a0->spawn(pi->world, 0x57899875c4457313);
    pi->viewport = ct_renderer_a0->create_viewport();

    ct_dock_a0->create_dock(RESOURCE_PREVIEW_I, true);
    return true;
}

static void update(float dt) {
    uint32_t n = ce_array_size(_G.instances);
    for (int i = 0; i < n; ++i) {
        struct preview_instance *pi = &_G.instances[i];
        ct_ecs_a0->simulate(pi->world, dt);

        ct_transform_comp *t = ct_ecs_a0->get_one(pi->world, TRANSFORM_COMPONENT,
                                                  pi->camera_ent);

        ct_camera_component *c = ct_ecs_a0->get_one(pi->world, CT_CAMERA_COMPONENT,
                                                    pi->camera_ent);

        ct_renderer_a0->viewport_render(pi->viewport,
                                        pi->world,
                                        (ct_camera_data_t0) {
                                                .world = t->world,
                                                .camera = *c,
                                        });
    }
}

void set_background_resource(ct_resource_id_t0 resource) {
    set_asset(_G.baground, resource.uid);
}

void draw_background_texture(ce_vec2_t size) {
    _draw_preview(_G.baground, size);
}

static struct ct_resource_preview_a0 asset_preview_api = {
        .set_background_resource = set_background_resource,
        .draw_background_texture = draw_background_texture,
};

struct ct_resource_preview_a0 *ct_resource_preview_a0 = &asset_preview_api;

static const char *dock_title() {
    return "Resource preview";
}

static const char *name(uint64_t dock) {
    return "asset_preview";
}

static uint64_t cdb_type() {
    return RESOURCE_PREVIEW_I;
};

static void open(uint64_t dock) {
    preview_instance *pi = _new_preview();

    pi->world = ct_ecs_a0->create_world();
    pi->camera_ent = ct_ecs_a0->spawn(pi->world, 0x57899875c4457313);
    pi->viewport = ct_renderer_a0->create_viewport(pi->world, pi->camera_ent);

    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), dock);
    ce_cdb_a0->set_ptr(w, PREVIEW_PTR, pi);
    ce_cdb_a0->write_commit(w);
}

static struct ct_dock_i0 dock_api = {
        .cdb_type = cdb_type,
        .display_title = dock_title,
        .name = name,
        .draw_ui = draw_dock,
        .draw_menu = draw_menu,
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

    api->register_api(DOCK_INTERFACE, &dock_api, sizeof(dock_api));
    api->register_api(CT_ASSET_PREVIEW_API, &asset_preview_api, sizeof(asset_preview_api));
    api->register_api(EDITOR_MODULE_INTERFACE, &ct_editor_module_api, sizeof(ct_editor_module_api));
}

void CE_MODULE_UNLOAD(asset_preview)(struct ce_api_a0 *api,
                                     int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
