#include "cetech/core/containers/map.inl"

#include <cetech/core/log/log.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/module/module.h>

#include <cetech/playground/action_manager.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);

using namespace celib;

#define PLAYGROUND_MODULE_NAME CT_ID64_0("action_manager")

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
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)