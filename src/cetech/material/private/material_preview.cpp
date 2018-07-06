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
#include <cetech/ecs/entity_property.h>
#include <corelib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>

#include <cetech/asset_preview/asset_preview.h>
#include <cetech/mesh_renderer/mesh_renderer.h>
#include <cetech/material/material.h>


//==============================================================================
// GLobals
//==============================================================================

#define _G MaterialPreviewGlobals
static struct _G {
    ct_entity ent;
} _G;




static int _init(struct ct_api_a0 *api) {
    CT_UNUSED(api);

    _G = {};


//    ct_asset_preview_a0->register_type_preview(
//            "material",
//            (ct_asset_preview_fce) {
//                    .load = load,
//                    .unload = unload
//            });

    return 1;
}

static void _shutdown() {

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