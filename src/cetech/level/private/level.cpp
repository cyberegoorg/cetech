//==============================================================================
// Includes
//==============================================================================

#include <cetech/api/api_system.h>
#include <cetech/entity/entity.h>
#include <cetech/resource/resource.h>
#include <cetech/transform/transform.h>
#include <cetech/os/memory.h>
#include <cetech/os/vio.h>
#include <cetech/hashlib/hashlib.h>
#include <cetech/level/level.h>

#include <cetech/module/module.h>
#include <cetech/yaml/ydb.h>
#include "celib/array.inl"

using namespace celib;

CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);

//==============================================================================
// Globals
//==============================================================================

#define _G LevelGlobals
static struct LevelGlobals {
} LevelGlobals;

//==============================================================================
// Public interface
//==============================================================================



    static ct_entity load(ct_world world,
                   uint64_t name) {

        return ct_entity_a0.spawn_level(world, name);
    }

static void destroy(ct_world world,
                 ct_entity level) {
        ct_entity_a0.destroy(world, &level, 1);
    }

static ct_entity entity_by_id(ct_entity level,
                           uint64_t id) {
        return ct_entity_a0.find_by_guid(level, id);
    }



//==============================================================================
// Module interface
//==============================================================================

    static ct_level_a0 _api = {
            .load_level = load,
            .destroy = destroy,
            .entity_by_id = entity_by_id,
    };

static void _init_api(ct_api_a0 *api) {
        api->register_api("ct_level_a0", &_api);
    }


static void _init(ct_api_a0 *api) {
        _init_api(api);

        _G = {};

    }

static void _shutdown() {
        _G = {};
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
            CETECH_GET_API(api, ct_world_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_yng_a0);
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