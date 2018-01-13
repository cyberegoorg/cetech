//==============================================================================
// Includes
//==============================================================================

#include <cetech/entity/entity.h>
#include <celib/blob.h>
#include "celib/map.inl"

#include "cetech/os/memory.h"
#include "cetech/config/config.h"
#include "cetech/api/api_system.h"
#include "cetech/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_world_a0);

using namespace celib;

//==============================================================================
// Globals
//==============================================================================

namespace {
#define _G ComponentMaagerGlobals
    static struct ComponentMaagerGlobals {
        Map<ct_component_compiler_t> compiler_map;
        Map<uint32_t> spawn_order_map;
        Map<ct_component_clb> component_clb;
    } ComponentMaagerGlobals;
}


//==============================================================================
// Public interface
//==============================================================================

namespace component {

    void register_compiler(uint64_t type,
                           ct_component_compiler_t compiler,
                           uint32_t spawn_order) {
        map::set(_G.compiler_map, type, compiler);
        map::set(_G.spawn_order_map, type, spawn_order);
    }

    int compile(uint64_t type,
                const char* filename,
                uint64_t* component_key,
                uint32_t component_key_count,
                ct_blob *data) {

        ct_component_compiler_t compiler = map::get<ct_component_compiler_t>(
                _G.compiler_map, type, nullptr);

        if (!compiler) {
            return 0;
        }

        return compiler(filename, component_key, component_key_count, data);
    }

    uint32_t get_spawn_order(uint64_t type) {
        return map::get(_G.spawn_order_map, type, (uint32_t) 0);
    }

    void register_type(uint64_t type,
                       ct_component_clb clb) {
        map::set(_G.component_clb, type, clb);

        ct_world_callbacks_t wclb = {
                .on_created = clb.world_clb.on_created,
                .on_destroy = clb.world_clb.on_destroy,
                .on_update = clb.world_clb.on_update,
        };

        ct_world_a0.register_callback(wclb);
    }

    void spawn(ct_world world,
               uint64_t type,
               ct_entity *ent_ids,
               uint32_t *cent,
               uint32_t *ents_parent,
               uint32_t ent_count,
               void *data) {

        ct_component_clb clb = map::get(_G.component_clb, type,
                                        ct_component_clb_null);

        if (!clb.spawner) {
            return;
        }

        clb.spawner(world, ent_ids, cent, ents_parent, ent_count, data);
    }

    void destroy(ct_world world,
                 ct_entity *ent,
                 uint32_t count) {

        auto ct_it = map::begin(_G.component_clb);
        auto ct_end = map::end(_G.component_clb);

        while (ct_it != ct_end) {
            ct_it->value.destroyer(world, ent, count);
            ++ct_it;
        }
    }
}

namespace component_module {
    static ct_component_a0 component_api = {
            .register_compiler = component::register_compiler,
            .compile = component::compile,
            .spawn_order = component::get_spawn_order,
            .register_type = component::register_type,
            .spawn = component::spawn,
            .destroy = component::destroy,
    };

    void _init_api(ct_api_a0 *a0) {
        a0->register_api("ct_component_a0", &component_api);
    }

    void _init(ct_api_a0 *a0) {
        _init_api(a0);

        _G = {};

        _G.compiler_map.init(ct_memory_a0.main_allocator());
        _G.spawn_order_map.init(ct_memory_a0.main_allocator());
        _G.component_clb.init(ct_memory_a0.main_allocator());
    }

    void _shutdown() {
        _G.compiler_map.destroy();
        _G.spawn_order_map.destroy();
        _G.component_clb.destroy();
    }
}

CETECH_MODULE_DEF(
        component,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_world_a0);
        },
        {
            CEL_UNUSED(reload);
            component_module::_init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            component_module::_shutdown();
        }
)

