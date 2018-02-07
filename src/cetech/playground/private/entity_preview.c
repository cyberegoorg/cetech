//==============================================================================
// Include
//==============================================================================

#include "cetech/core/memory/allocator.h"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/log/log.h"
#include <cetech/core/module/module.h>
#include <cetech/playground/entity_property.h>
#include <cetech/engine/world/world.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/playground/asset_preview.h>

//==============================================================================
// GLobals
//==============================================================================

#define _G EntityPreviewGlobals
static struct _G {
    struct ct_entity ent;
} _G;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_asset_preview_a0);
CETECH_DECL_API(ct_world_a0);

static void load(const char *filename,
                 uint64_t type,
                 uint64_t name,
                 struct ct_world world) {
    struct ct_entity ent = ct_world_a0.spawn_entity(world, name);
    _G.ent = ent;
}

static void unload(const char *filename,
                   uint64_t type,
                   uint64_t name,
                   struct ct_world world) {
    ct_world_a0.destroy_entity(world, &_G.ent, 1);
}

static int _init(struct ct_api_a0 *api) {
    CT_UNUSED(api);

    _G = (struct _G) {};


    ct_asset_preview_a0.register_type_preview(
            CT_ID64_0("entity"),
            (struct ct_asset_preview_fce) {
                    .load = load,
                    .unload = unload
            });

    return 1;
}

static void _shutdown() {
    ct_asset_preview_a0.unregister_type_preview(CT_ID64_0("entity"));

    _G = (struct _G) {};
}


CETECH_MODULE_DEF(
        entity_preview,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_asset_preview_a0);
            CETECH_GET_API(api, ct_world_a0);

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