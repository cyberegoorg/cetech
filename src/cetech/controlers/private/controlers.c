//==============================================================================
// Includes
//==============================================================================
#include <string.h>

#include <corelib/api_system.h>
#include <corelib/log.h>
#include <corelib/module.h>
#include <corelib/ebus.h>
#include <corelib/hashlib.h>
#include <corelib/macros.h>
#include <corelib/os.h>
#include "corelib/allocator.h"

#include <cetech/machine/machine.h>
#include <cetech/kernel/kernel.h>

#import <cetech/controlers/controlers.h>
#include <corelib/memory.h>
#include <corelib/hash.inl>

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "controlers"


//==============================================================================
// Globals
//==============================================================================

#define _G controlers_global
static struct _G {
    struct ct_hash_t interface_map;
} _G = {};


//==============================================================================
// Interface
//==============================================================================

static void _componet_api_add(uint64_t name,
                              void *api) {
    struct ct_controlers_i0 *controlers_i = api;


    ct_hash_add(&_G.interface_map, controlers_i->name(),
                (uint64_t) controlers_i, ct_memory_a0->system);
}

struct ct_controlers_i0* get_by_name(uint64_t name) {
    struct ct_controlers_i0 *controlers_i;
    controlers_i = (struct ct_controlers_i0 *) ct_hash_lookup(&_G.interface_map, name, 0);
    return controlers_i;
};

static struct ct_controlers_a0 ct_controlers_api = {
        .get_by_name = get_by_name,
};

struct ct_controlers_a0 *ct_controlers_a0 = &ct_controlers_api;


static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_controlers_a0", ct_controlers_a0);
}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);
    _G = (struct _G) {};

    ct_api_a0->register_on_add(CT_ID64_0("ct_controlers_i0"), _componet_api_add);

}

static void _shutdown() {
    ct_log_a0->debug(LOG_WHERE, "Shutdown");

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        controlers,
        {
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_cdb_a0);
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