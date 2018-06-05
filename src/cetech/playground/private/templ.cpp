#include "cetech/kernel/containers/map.inl"

#include <cetech/kernel/log/log.h>
#include <cetech/kernel/hashlib/hashlib.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/module/module.h>

#include <cetech/playground/action_manager.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);

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
            CETECH_GET_API(api, ct_hashlib_a0);
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