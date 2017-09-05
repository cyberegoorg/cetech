//==============================================================================
// Includes
//==============================================================================

#include <cetech/kernel/api_system.h>
#include <cetech/modules/entity/entity.h>
#include <cetech/kernel/resource.h>
#include <cetech/modules/transform/transform.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/vio.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/modules/level/level.h>

#include <cetech/kernel/module.h>
#include <cetech/kernel/blob.h>
#include <cetech/kernel/ydb.h>
#include "celib/array.inl"

using namespace celib;

CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_blob_a0);
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yamlng_a0);

//==============================================================================
// Globals
//==============================================================================

#define _G LevelGlobals
static struct LevelGlobals {
} LevelGlobals;

//==============================================================================
// Public interface
//==============================================================================


namespace level {

    ct_entity load(ct_world world,
                   uint64_t name) {

        return ct_entity_a0.spawn_level(world, name);
    }

    void destroy(ct_world world,
                 ct_entity level) {
        ct_entity_a0.destroy(world, &level, 1);
    }

    ct_entity entity_by_id(ct_entity level,
                           uint64_t id) {
        return ct_entity_a0.find_by_guid(level, id);
    }

}


//==============================================================================
// Module interface
//==============================================================================

namespace level_module {
    static ct_level_a0 _api = {
            .load_level = level::load,
            .destroy = level::destroy,
            .entity_by_id = level::entity_by_id,
    };

    void _init_api(ct_api_a0 *api) {
        api->register_api("ct_level_a0", &_api);
    }


    void _init(ct_api_a0 *api) {
        _init_api(api);

        _G = {};

    }

    void _shutdown() {
        _G = {};
    }
}

CETECH_MODULE_DEF(
        level,
        {
            CETECH_GET_API(api, ct_entity_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_blob_a0);
            CETECH_GET_API(api, ct_world_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_yamlng_a0);
        },
        {
            CEL_UNUSED(reload);
            level_module::_init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            level_module::_shutdown();

        }
)