//==============================================================================
// Include
//==============================================================================

#include "corelib/allocator.h"
#include "corelib/buffer.inl"

#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/log.h"
#include <corelib/module.h>
#include <cetech/playground/entity_property.h>
#include <corelib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>

#include <cetech/playground/asset_preview.h>
#include <cetech/mesh_renderer/mesh_renderer.h>
#include <cetech/material/material.h>


//==============================================================================
// GLobals
//==============================================================================

#define _G MaterialPreviewGlobals
static struct _G {
    ct_entity ent;
} _G;


static void load(const char *filename,
                 struct ct_resource_id resourceid,
                 struct ct_world world) {
    ct_entity ent = ct_ecs_a0->spawn_entity(world, CT_ID32_0("core/cube"));
    _G.ent = ent;

    uint64_t obj = ct_ecs_a0->entity_object(world, ent);
    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(obj);
    ct_cdb_a0->set_ref(w, PROP_MATERIAL_ID, resourceid.name);
    ct_cdb_a0->write_commit(w);
}

static void unload(const char *filename,
                   struct ct_resource_id resourceid,
                   struct ct_world world) {
    if (_G.ent.h != 0) {
        ct_ecs_a0->destroy_entity(world, &_G.ent, 1);
    }
}

static int _init(struct ct_api_a0 *api) {
    CT_UNUSED(api);

    _G = {};


    ct_asset_preview_a0->register_type_preview(
            "material",
            (ct_asset_preview_fce) {
                    .load = load,
                    .unload = unload
            });

    return 1;
}

static void _shutdown() {
    ct_asset_preview_a0->unregister_type_preview("material");

    _G = {};
}


CETECH_MODULE_DEF(
        material_preview,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_asset_preview_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_mesh_renderer_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_material_a0);

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