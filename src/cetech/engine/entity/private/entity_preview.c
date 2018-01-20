//==============================================================================
// Include
//==============================================================================

#include "cetech/core/allocator.h"

#include "cetech/core/hashlib.h"
#include "cetech/core/memory.h"
#include "cetech/core/api_system.h"
#include "cetech/core/log.h"
#include <cetech/core/module.h>
#include <cetech/playground/entity_property.h>
#include <cetech/engine/entity/entity.h>
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
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_asset_preview_a0);
CETECH_DECL_API(ct_entity_a0);

static void load(const char *filename,
                 uint64_t type,
                 uint64_t name,
                 struct ct_world world) {
    struct ct_entity ent = ct_entity_a0.spawn(world, name);
    _G.ent = ent;

    if (ct_transform_a0.has(world, ent)) {
        struct ct_transform t = ct_transform_a0.get(world, ent);
        ct_transform_a0.set_position(t, (float[3]) {0.0f});
    }
}

static void unload(const char *filename,
                   uint64_t type,
                   uint64_t name,
                   struct ct_world world) {
    ct_entity_a0.destroy(world, &_G.ent, 1);
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
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_asset_preview_a0);
            CETECH_GET_API(api, ct_entity_a0);

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