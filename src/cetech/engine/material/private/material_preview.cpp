//==============================================================================
// Include
//==============================================================================

#include "cetech/kernel/memory/allocator.h"
#include "cetech/kernel/containers/map.inl"
#include "cetech/kernel/containers/buffer.h"

#include "cetech/kernel/hashlib/hashlib.h"
#include "cetech/kernel/memory/memory.h"
#include "cetech/kernel/api/api_system.h"
#include "cetech/kernel/log/log.h"
#include <cetech/kernel/module/module.h>
#include <cetech/playground//entity_property.h>
#include <cetech/engine/ecs/ecs.h>
#include <cetech/engine/transform/transform.h>

#include <cetech/playground/asset_preview.h>
#include <cetech/engine/mesh_renderer/mesh_renderer.h>
#include <cetech/engine/material/material.h>

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
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_mesh_renderer_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_material_a0);


static void load(const char *filename,
                 struct ct_resource_id resourceid,
                 struct ct_world world) {
    ct_entity ent = ct_ecs_a0.spawn_entity(world, CT_ID32_0("kernel/cube"));
    _G.ent = ent;

    ct_mesh_renderer *component = static_cast<ct_mesh_renderer *>(ct_ecs_a0.entity_data(world, MESH_RENDERER_COMPONENT, ent));

    component->geometries[0].material = ct_material_a0.resource_create(resourceid.name);
    ct_ecs_a0.entity_component_change(world, MESH_RENDERER_COMPONENT, ent);
}

static void unload(const char *filename,
                   struct ct_resource_id resourceid,
                   struct ct_world world) {
    if (_G.ent.h != 0) {
        ct_ecs_a0.destroy_entity(world, &_G.ent, 1);
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