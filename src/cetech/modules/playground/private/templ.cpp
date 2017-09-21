#include "celib/map.inl"

#include <cetech/kernel/log.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/module.h>

#include <cetech/modules/playground/action_manager.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);

using namespace celib;

#define PLAYGROUND_MODULE_NAME ct_hash_a0.id64_from_str("action_manager")

#define _G action_manager_global
static struct _G {
} _G;

static ct_action_manager_a0 action_manager_api = {
};


static void _init(ct_api_a0 *api) {
    _G = {
    };

    api->register_api("ct_action_manager_a0", &action_manager_api);
}

static void _shutdown() {
    _G = {};
}

CETECH_MODULE_DEF(
        action_manager,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
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