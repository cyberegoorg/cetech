//==============================================================================
// Include
//==============================================================================

#include "cetech/core/memory/allocator.h"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/log/log.h"
#include "cetech/core/os/path.h"
#include "cetech/core/os/vio.h"
#include "cetech/engine/resource/resource.h"
#include <cetech/core/module/module.h>
#include <cetech/playground/asset_property.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/renderer/texture.h>
#include <cetech/playground//entity_property.h>
#include <cetech/engine/entity/entity.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/playground/asset_preview.h>

//==============================================================================
// GLobals
//==============================================================================

#define _G LevelPreviewGlobals
static struct _G {
    struct ct_entity ent;
} _G;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_asset_property_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_texture_a0);
CETECH_DECL_API(ct_entity_property_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_asset_preview_a0);
CETECH_DECL_API(ct_entity_a0);

static void load(const char *filename,
                 uint64_t type,
                 uint64_t name,
                 struct ct_world world) {
    _G.ent = ct_entity_a0.spawn_level(world, name);
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
            CT_ID64_0("level"),
            (struct ct_asset_preview_fce) {
                    .load = load,
                    .unload = unload
            });

    return 1;
}

static void _shutdown() {
    ct_asset_preview_a0.unregister_type_preview(CT_ID64_0("level"));

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        level_preview,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_asset_property_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_texture_a0);
            CETECH_GET_API(api, ct_entity_property_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_yng_a0);
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