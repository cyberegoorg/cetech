//==============================================================================
// Includes
//==============================================================================

#include "celib/map.h"
#include <cetech/memory/memory.h>
#include <cetech/module/module.h>

#include <cetech/world/world.h>
#include <cetech/component/component.h>

//==============================================================================
// Globals
//==============================================================================

ARRAY_PROTOTYPE(component_compiler_t);
ARRAY_PROTOTYPE_N(struct component_clb, component_clb_t);

MAP_PROTOTYPE(component_compiler_t);
MAP_PROTOTYPE_N(struct component_clb, component_clb_t);


#define _G ComponentMaagerGlobals
static struct G {
    MAP_T(component_compiler_t) compiler_map;
    MAP_T(u32) spawn_order_map;

    MAP_T(component_clb_t) component_clb;
} _G = {0};

IMPORT_API(MemSysApi, 0);
IMPORT_API(WorldApi, 0);

static void _init(get_api_fce_t get_engine_api) {
    INIT_API(MemSysApi, MEMORY_API_ID, 0);
    INIT_API(WorldApi, WORLD_API_ID, 0);

    _G = (struct G) {0};

    MAP_INIT(component_compiler_t, &_G.compiler_map,
             MemSysApiV0.main_allocator());
    MAP_INIT(u32, &_G.spawn_order_map, MemSysApiV0.main_allocator());
    MAP_INIT(component_clb_t, &_G.component_clb, MemSysApiV0.main_allocator());
}

static void _shutdown() {
    MAP_DESTROY(component_compiler_t, &_G.compiler_map);
    MAP_DESTROY(u32, &_G.spawn_order_map);
    MAP_DESTROY(component_clb_t, &_G.component_clb);

    _G = (struct G) {0};
}

//==============================================================================
// Public interface
//==============================================================================

void component_register_compiler(stringid64_t type,
                                 component_compiler_t compiler,
                                 u32 spawn_order) {
    MAP_SET(component_compiler_t, &_G.compiler_map, type.id, compiler);
    MAP_SET(u32, &_G.spawn_order_map, type.id, spawn_order);
}

int component_compile(stringid64_t type,
                      yaml_node_t body,
                      ARRAY_T(u8) *data) {

    component_compiler_t compiler = MAP_GET(component_compiler_t,
                                            &_G.compiler_map, type.id, NULL);

    if (!compiler) {
        return 0;
    }

    return compiler(body, data);
}

u32 component_get_spawn_order(stringid64_t type) {
    return MAP_GET(u32, &_G.spawn_order_map, type.id, 0);
}

void component_register_type(stringid64_t type,
                             struct component_clb clb) {
    MAP_SET(component_clb_t, &_G.component_clb, type.id, clb);

    world_callbacks_t wclb = {
            .on_created = clb.on_world_create,
            .on_destroy = clb.on_world_destroy,
            .on_update = clb.on_world_update,
    };

    WorldApiV0.register_callback(wclb);
}

void component_spawn(world_t world,
                     stringid64_t type,
                     entity_t *ent_ids,
                     u32 *cent,
                     u32 *ents_parent,
                     u32 ent_count,
                     void *data) {

    struct component_clb clb = MAP_GET(component_clb_t, &_G.component_clb,
                                       type.id, component_clb_null);

    if (!clb.spawner) {
        return;
    }

    clb.spawner(world, ent_ids, cent, ents_parent, ent_count, data);
}

void component_destroy(world_t world,
                       entity_t *ent,
                       u32 count) {

    const MAP_ENTRY_T(component_clb_t) *ce_it = MAP_BEGIN(component_clb_t,
                                                          &_G.component_clb);
    const MAP_ENTRY_T(component_clb_t) *ce_end = MAP_END(component_clb_t,
                                                         &_G.component_clb);
    while (ce_it != ce_end) {
        ce_it->value.destroyer(world, ent, count);
        ++ce_it;
    }
}

void *component_get_module_api(int api,
                            int version) {
    switch (api) {
        case PLUGIN_EXPORT_API_ID:
            switch (version) {
                case 0: {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;


                    return &module;
                }

                default:
                    return NULL;
            };

        case COMPONENT_API_ID:
            switch (version) {
                case 0: {
                    static struct ComponentSystemApiV0 api = {0};
                    api.component_register_compiler = component_register_compiler;
                    api.component_compile = component_compile;
                    api.component_get_spawn_order = component_get_spawn_order;
                    api.component_register_type = component_register_type;
                    api.component_spawn = component_spawn;
                    api.component_destroy = component_destroy;

                    return &api;
                }

                default:
                    return NULL;
            };

        default:
            return NULL;
    }
}