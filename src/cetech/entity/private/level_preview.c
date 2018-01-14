//==============================================================================
// Include
//==============================================================================

#include "celib/allocator.h"

#include "cetech/hashlib/hashlib.h"
#include "cetech/os/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/log/log.h"
#include "cetech/os/path.h"
#include "cetech/os/vio.h"
#include "cetech/resource/resource.h"
#include <cetech/module/module.h>
#include <cetech/playground/asset_property.h>
#include <cetech/debugui/debugui.h>
#include <cetech/renderer/texture.h>
#include <cetech/playground//entity_property.h>
#include <cetech/entity/entity.h>
#include <cetech/transform/transform.h>
#include <cetech/yaml/ydb.h>
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
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_asset_property_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_texture_a0);
CETECH_DECL_API(ct_entity_property_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_asset_preview_a0);
CETECH_DECL_API(ct_entity_a0);

static void load(const char* filename, uint64_t type, uint64_t name, struct ct_world world){
    _G.ent = ct_entity_a0.spawn_level(world, name);
}

static void unload(const char* filename, uint64_t type, uint64_t name, struct ct_world world){
    ct_entity_a0.destroy(world, &_G.ent, 1);
}

static int _init(struct ct_api_a0 *api) {
    CEL_UNUSED(api);

    _G = (struct _G){};

    ct_asset_preview_a0.register_type_preview(
            CT_ID64_0("level"),
            (struct ct_asset_preview_fce){
                    .load = load,
                    .unload = unload
            });

    return 1;
}

static void _shutdown() {
    ct_asset_preview_a0.unregister_type_preview(CT_ID64_0("level"));

    _G = (struct _G){};
}

CETECH_MODULE_DEF(
        level_preview,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hash_a0);
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
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            _shutdown();
        }
)