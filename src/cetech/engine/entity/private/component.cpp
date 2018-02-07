//==============================================================================
// Includes
//==============================================================================

#include <cetech/engine/entity/entity.h>
#include <cetech/core/containers/hash.h>

#include "cetech/core/memory/memory.h"
#include "cetech/core/config/config.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_world_a0);


//==============================================================================
// Globals
//==============================================================================

#define _G ComponentMaagerGlobals
static struct _G {
    ct_hash_t spawn_order_map;

    ct_hash_t compiler_map;
    ct_hash_t component_clb_map;

    ct_component_compiler_t *compilers;

    ct_alloc *allocator;
} _G;


//==============================================================================
// Public interface
//==============================================================================

static void register_compiler(uint64_t type,
                              ct_component_compiler_t compiler) {
    ct_array_push(_G.compilers, compiler, _G.allocator);

    ct_hash_add(&_G.compiler_map, type, ct_array_size(_G.compilers) - 1,
                 _G.allocator);
}

static int compile(uint64_t type,
                   const char *filename,
                   uint64_t *component_key,
                   uint32_t component_key_count,
                   struct ct_cdb_writer_t *writer) {

    uint64_t idx = ct_hash_lookup(&_G.compiler_map, type, UINT64_MAX);
    if (idx == UINT64_MAX) {
        return 0;
    }

    ct_component_compiler_t compiler = _G.compilers[idx];
    return compiler(filename, component_key, component_key_count, writer);
}

static ct_component_a0 component_api = {
        .compile = compile,
        .register_compiler = register_compiler
};

static void _init_api(ct_api_a0 *a0) {
    a0->register_api("ct_component_a0", &component_api);
}

static void _init(ct_api_a0 *a0) {
    _init_api(a0);

    _G = {
            .allocator = ct_memory_a0.main_allocator()
    };

}

static void _shutdown() {
    ct_hash_free(&_G.compiler_map, _G.allocator);
    ct_hash_free(&_G.component_clb_map, _G.allocator);
    ct_hash_free(&_G.spawn_order_map, _G.allocator);
}

CETECH_MODULE_DEF(
        component,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_world_a0);
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

