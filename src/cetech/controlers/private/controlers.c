//==============================================================================
// Includes
//==============================================================================
#include <string.h>

#include <celib/api.h>
#include <celib/log.h>
#include <celib/module.h>
#include <celib/cdb.h>

#include <celib/id.h>
#include <celib/macros.h>

#include "celib/memory/allocator.h"
#include <celib/memory/memory.h>
#include <celib/containers/hash.h>

#include <cetech/machine/machine.h>
#include <cetech/kernel/kernel.h>
#import <cetech/controlers/controlers.h>


//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "controlers"

//==============================================================================
// Globals
//==============================================================================

#define _G controlers_global
static struct _G {
    ce_hash_t interface_map;
} _G = {};


//==============================================================================
// Interface
//==============================================================================

static void _componet_api_add(uint64_t name,
                              void *api) {
    ct_controler_i0 *controlers_i = api;


    ce_hash_add(&_G.interface_map, controlers_i->name(),
                (uint64_t) controlers_i, ce_memory_a0->system);
}

struct ct_controler_i0 *get_by_name(uint64_t name) {
    ct_controler_i0 *controlers_i;
    controlers_i = (ct_controler_i0 *) ce_hash_lookup(&_G.interface_map, name, 0);
    return controlers_i;
};

static struct ct_controlers_a0 ct_controlers_api = {
        .get = get_by_name,
};

struct ct_controlers_a0 *ct_controlers_a0 = &ct_controlers_api;

void CE_MODULE_LOAD(controlers)(struct ce_api_a0 *api,
                                int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);

    _G = (struct _G) {};

    api->register_api(CT_CONTROLERS_API,
                      ct_controlers_a0, sizeof(ct_controlers_api));

    ce_api_a0->register_on_add(CT_CONTROLERS_I, _componet_api_add);
}

void CE_MODULE_UNLOAD(controlers)(struct ce_api_a0 *api,
                                  int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _G = (struct _G) {};
}