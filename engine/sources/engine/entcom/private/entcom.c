//==============================================================================
// Includes
//==============================================================================

#include "celib/containers/map.h"

#include "engine/world/world.h"
#include "celib/memory/memory.h"

#include "engine/entcom/entcom.h"

//==============================================================================
// Globals
//==============================================================================

ARRAY_PROTOTYPE(component_compiler_t);
ARRAY_PROTOTYPE(component_spawner_t);
ARRAY_PROTOTYPE(component_destroyer_t);
ARRAY_PROTOTYPE(component_on_world_create_t);
ARRAY_PROTOTYPE(component_on_world_destroy_t);

MAP_PROTOTYPE(component_compiler_t);
MAP_PROTOTYPE(component_spawner_t);
MAP_PROTOTYPE(component_destroyer_t);
MAP_PROTOTYPE(component_on_world_create_t);
MAP_PROTOTYPE(component_on_world_destroy_t);

#define _G EntityMaagerGlobals
static struct G {
    struct handlerid entity_handler;
    MAP_T(component_compiler_t) compiler_map;
    MAP_T(u32) spawn_order_map;

    MAP_T(component_spawner_t) spawner_map;
    MAP_T(component_destroyer_t) destroyer_map;
    MAP_T(component_on_world_create_t) on_world_create_map;
    MAP_T(component_on_world_destroy_t) on_world_destroy_map;
} _G = {0};


//==============================================================================
// Public interface
//==============================================================================

int entcom_init(int stage) {
    if (stage == 0) {
        return 1;
    }

    _G = (struct G) {0};

    handlerid_init(&_G.entity_handler, memsys_main_allocator());
    MAP_INIT(component_compiler_t, &_G.compiler_map, memsys_main_allocator());
    MAP_INIT(u32, &_G.spawn_order_map, memsys_main_allocator());
    MAP_INIT(component_spawner_t, &_G.spawner_map, memsys_main_allocator());
    MAP_INIT(component_destroyer_t, &_G.destroyer_map, memsys_main_allocator());
    MAP_INIT(component_on_world_create_t, &_G.on_world_create_map, memsys_main_allocator());
    MAP_INIT(component_on_world_destroy_t, &_G.on_world_destroy_map, memsys_main_allocator());

//    CE_ASSERT("entcom", handlerid_handler_create(&_G.entity_handler).h == 0);

    return 1;
}

void entcom_shutdown() {
    handlerid_destroy(&_G.entity_handler);
    MAP_DESTROY(component_compiler_t, &_G.compiler_map);
    MAP_DESTROY(u32, &_G.spawn_order_map);
    MAP_DESTROY(component_spawner_t, &_G.spawner_map);
    MAP_DESTROY(component_destroyer_t, &_G.destroyer_map);
    MAP_DESTROY(component_on_world_create_t, &_G.on_world_create_map);
    MAP_DESTROY(component_on_world_destroy_t, &_G.on_world_destroy_map);

    _G = (struct G) {0};
}

entity_t entity_manager_create() {
    return (entity_t) {.idx = handlerid_handler_create(&_G.entity_handler).h};
}

void entity_manager_destroy(entity_t entity) {
    handlerid_handler_destroy(&_G.entity_handler, entity.h);
}

int entity_manager_alive(entity_t entity) {
    return handlerid_handler_alive(&_G.entity_handler, entity.h);
}


void component_register_compiler(stringid64_t type,
                                 component_compiler_t compiler,
                                 u32 spawn_order) {
    MAP_SET(component_compiler_t, &_G.compiler_map, type.id, compiler);
    MAP_SET(u32, &_G.spawn_order_map, type.id, spawn_order);
}

int component_compile(stringid64_t type,
                      yaml_node_t body,
                      ARRAY_T(u8) *data) {

    component_compiler_t compiler = MAP_GET(component_compiler_t, &_G.compiler_map, type.id, NULL);

    if (!compiler) {
        return 0;
    }

    return compiler(body, data);
}

u32 component_get_spawn_order(stringid64_t type) {
    return MAP_GET(u32, &_G.spawn_order_map, type.id, 0);
}

void component_register_type(stringid64_t type,
                             component_spawner_t spawner,
                             component_destroyer_t destroyer,
                             component_on_world_create_t on_world_create,
                             component_on_world_destroy_t on_world_destroy) {

    MAP_SET(component_spawner_t, &_G.spawner_map, type.id, spawner);
    MAP_SET(component_destroyer_t, &_G.destroyer_map, type.id, destroyer);
    MAP_SET(component_on_world_create_t, &_G.on_world_create_map, type.id, on_world_create);
    MAP_SET(component_on_world_destroy_t, &_G.on_world_destroy_map, type.id, on_world_destroy);

    world_callbacks_t clb = {
            .on_created = (world_on_created_t) on_world_create,
            .on_destroy = (world_on_destroy_t) on_world_destroy,
    };

    world_register_callback(clb);
}

void component_spawn(world_t world,
                     stringid64_t type,
                     entity_t *ent_ids,
                     u32 *cent,
                     u32 *ents_parent,
                     u32 ent_count,
                     void *data) {
    component_spawner_t clb = MAP_GET(component_spawner_t, &_G.spawner_map, type.id, NULL);

    if (clb == NULL) {
        return;
    }

    clb(world, ent_ids, cent, ents_parent, ent_count, data);

}

void component_destroy(world_t world,
                       entity_t *ent,
                       u32 count) {
    const MAP_ENTRY_T(component_destroyer_t) *ce_it = MAP_BEGIN(component_destroyer_t, &_G.destroyer_map);
    const MAP_ENTRY_T(component_destroyer_t) *ce_end = MAP_END(component_destroyer_t, &_G.destroyer_map);
    while (ce_it != ce_end) {
        ce_it->value(world, ent, count);
        ++ce_it;
    }
}
