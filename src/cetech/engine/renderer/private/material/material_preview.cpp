//==============================================================================
// Include
//==============================================================================

#include "cetech/core/memory/allocator.h"
#include "cetech/core/containers/map.inl"
#include "cetech/core/containers/buffer.h"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/log/log.h"
#include <cetech/core/module/module.h>
#include <cetech/playground//entity_property.h>
#include <cetech/engine/world/world.h>
#include <cetech/engine/transform/transform.h>

#include <cetech/playground/asset_preview.h>
#include <cetech/engine/renderer/mesh_renderer.h>
#include <cetech/engine/renderer/material.h>

using namespace celib;

//==============================================================================
// GLobals
//==============================================================================

#define _G MaterialPreviewGlobals
static struct _G {
    ct_entity ent;
} _G;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_asset_preview_a0);
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_mesh_renderer_a0);
CETECH_DECL_API(ct_cdb_a0);


static void load(const char *filename,
                 struct ct_resource_id resourceid,
                 struct ct_world world) {
    ct_entity ent = ct_world_a0.spawn_entity(world, CT_ID32_0("core/cube"));
    _G.ent = ent;

    ct_cdb_obj_t*w  = ct_cdb_a0.write_begin(ct_world_a0.ent_obj(world, ent));
    ct_cdb_a0.set_uint64(w, PROP_MATERIAL_ID, resourceid.name);
    ct_cdb_a0.write_commit(w);
}

static void unload(const char *filename,
                   struct ct_resource_id resourceid,
                   struct ct_world world) {
    if (_G.ent.h != 0) {
        ct_world_a0.destroy_entity(world, &_G.ent, 1);
    }
}

static int _init(ct_api_a0 *api) {
    CT_UNUSED(api);

    _G = {};


    ct_asset_preview_a0.register_type_preview(
            "material",
            (ct_asset_preview_fce) {
                    .load = load,
                    .unload = unload
            });

    return 1;
}

static void _shutdown() {
    ct_asset_preview_a0.unregister_type_preview("material");

    _G = {};
}


CETECH_MODULE_DEF(
        material_preview,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_asset_preview_a0);
            CETECH_GET_API(api, ct_world_a0);
            CETECH_GET_API(api, ct_mesh_renderer_a0);
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