//==============================================================================
// Include
//==============================================================================
#include <celib/ydb.h>
#include <celib/os.h>
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/hashlib.h"
#include <celib/array.inl>
#include <celib/ydb.h>
#include <celib/cdb.h>

#include "cetech/ecs/ecs.h"
#include "cetech/machine/machine.h"
#include <bgfx/defines.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include "cetech/resource/resource.h"

#include <cetech/material/material.h>
#include <cetech/resource/builddb.h>
#include <celib/buffer.inl>
#include "material.h"

#define _G material_compiler_globals

struct _G {
    struct ce_alloc *allocator;
} _G;


uint64_t material_compiler(uint64_t k,
                           struct ct_resource_id rid,
                           const char *fullname) {

    uint64_t obj = k;

//    if (ce_cdb_a0->prop_exist(k, ce_ydb_a0->key("layers"))) {
//
//        uint64_t layers = ce_cdb_a0->read_subobject(k, ce_ydb_a0->key("layers"),
//                                                    0);
//        const uint64_t layers_keys_count = ce_cdb_a0->prop_count(layers);
//        uint64_t layers_keys[layers_keys_count];
//        ce_cdb_a0->prop_keys(layers, layers_keys);
//
//        for (uint32_t i = 0; i < layers_keys_count; ++i) {
//            uint64_t _layer = ce_cdb_a0->read_subobject(layers, layers_keys[i],
//                                                        0);
//
//            foreach_layer(filename, _layer, layers_keys[i], layer_obj);
//        }
//        ce_cdb_a0->write_commit(w);
//    }

    return obj;
}

int materialcompiler_init(struct ce_api_a0 *api) {
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_os_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_ydb_a0);

    _G = (struct _G) {
            .allocator=ce_memory_a0->system
    };


    return 1;
}
