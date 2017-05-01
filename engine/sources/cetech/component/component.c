//==============================================================================
// Includes
//==============================================================================

#include "cetech/map.inl"
#include "cetech/math_vec3f.inl"

#include <cetech/memory.h>
#include <cetech/application.h>
#include <cetech/config.h>
#include <cetech/vio.h>
#include <cetech/resource.h>

#include <cetech/module.h>
#include <cetech/stringid.h>

#include <cetech/world.h>
#include <cetech/entity.h>
#include <cetech/component.h>
#include <cetech/yaml.h>

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
    MAP_T(uint32_t) spawn_order_map;

    MAP_T(component_clb_t) component_clb;
} _G = {0};

IMPORT_API(memory_api_v0);
IMPORT_API(world_api_v0);

static void _init(get_api_fce_t get_engine_api) {
    INIT_API(get_engine_api, memory_api_v0, MEMORY_API_ID);
    INIT_API(get_engine_api, world_api_v0, WORLD_API_ID);

    _G = (struct G) {0};

    MAP_INIT(component_compiler_t, &_G.compiler_map,
             memory_api_v0.main_allocator());
    MAP_INIT(uint32_t, &_G.spawn_order_map, memory_api_v0.main_allocator());
    MAP_INIT(component_clb_t, &_G.component_clb, memory_api_v0.main_allocator());
}

static void _shutdown() {
    MAP_DESTROY(component_compiler_t, &_G.compiler_map);
    MAP_DESTROY(uint32_t, &_G.spawn_order_map);
    MAP_DESTROY(component_clb_t, &_G.component_clb);

    _G = (struct G) {0};
}

//==============================================================================
// Public interface
//==============================================================================

void component_register_compiler(stringid64_t type,
                                 component_compiler_t compiler,
                                 uint32_t spawn_order) {
    MAP_SET(component_compiler_t, &_G.compiler_map, type.id, compiler);
    MAP_SET(uint32_t, &_G.spawn_order_map, type.id, spawn_order);
}

int component_compile(stringid64_t type,
                      yaml_node_t body,
                      ARRAY_T(uint8_t) *data) {

    component_compiler_t compiler = MAP_GET(component_compiler_t,
                                            &_G.compiler_map, type.id, NULL);

    if (!compiler) {
        return 0;
    }

    return compiler(body, data);
}

uint32_t component_get_spawn_order(stringid64_t type) {
    return MAP_GET(uint32_t, &_G.spawn_order_map, type.id, 0);
}

void component_register_type(stringid64_t type,
                             struct component_clb clb) {
    MAP_SET(component_clb_t, &_G.component_clb, type.id, clb);

    world_callbacks_t wclb = {
            .on_created = clb.on_world_create,
            .on_destroy = clb.on_world_destroy,
            .on_update = clb.on_world_update,
    };

    world_api_v0.register_callback(wclb);
}

void component_spawn(world_t world,
                     stringid64_t type,
                     entity_t *ent_ids,
                     uint32_t *cent,
                     uint32_t *ents_parent,
                     uint32_t ent_count,
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
                       uint32_t count) {

    const MAP_ENTRY_T(component_clb_t) *ce_it = MAP_BEGIN(component_clb_t,
                                                          &_G.component_clb);
    const MAP_ENTRY_T(component_clb_t) *ce_end = MAP_END(component_clb_t,
                                                         &_G.component_clb);
    while (ce_it != ce_end) {
        ce_it->value.destroyer(world, ent, count);
        ++ce_it;
    }
}

static void _set_property(stringid64_t type,
                   world_t world,
                   entity_t entity,
                   stringid64_t key,
                   struct property_value value) {

    struct component_clb clb = MAP_GET(component_clb_t, &_G.component_clb,
                                       type.id, component_clb_null);

    if(!clb.set_property){
        return;
    }

    clb.set_property(world, entity, key, value);
}

static struct property_value _get_property(stringid64_t type,
                                    world_t world,
                                    entity_t entity,
                                    stringid64_t key) {

    struct property_value value = {PROPERTY_INVALID};

    struct component_clb clb = MAP_GET(component_clb_t, &_G.component_clb,
                                       type.id, component_clb_null);

    if(!clb.get_property){
        return (struct property_value){PROPERTY_INVALID};
    }

    return clb.get_property(world, entity, key);
}void *component_get_module_api(int api) {
    switch (api) {
        case PLUGIN_EXPORT_API_ID:
                {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;


                    return &module;
                }

        case COMPONENT_API_ID:
                {
                    static struct component_api_v0 api = {0};
                    api.component_register_compiler = component_register_compiler;
                    api.component_compile = component_compile;
                    api.component_get_spawn_order = component_get_spawn_order;
                    api.component_register_type = component_register_type;
                    api.component_spawn = component_spawn;
                    api.component_destroy = component_destroy;
                    api.set_property = _set_property;
                    api.get_property = _get_property;

                    return &api;
                }

        default:
            return NULL;
    }
}