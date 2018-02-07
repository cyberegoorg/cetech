//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/memory/memory.h>
#include <cetech/engine/world/world.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/core/os/path.h>
#include <cetech/core/log/log.h>
#include <cetech/core/os/vio.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/os/errors.h>
#include <cetech/core/module/module.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/core/containers/hash.h>
#include <cetech/core/containers/handler.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_cdb_a0);

//==============================================================================
// Globals
//==============================================================================

#define _G EntityMaagerGlobals

struct entity_instance {
    struct ct_world world;
    struct ct_entity *entity;
    uint64_t *uid;
    uint32_t entity_count;
    uint64_t name;
};

struct type_item {
    uint64_t mask;
    struct ct_hash_t ent_map;
    struct ct_cdb_obj_t **objects;
    struct ct_entity *entity;
};

struct world_instance {
    struct ct_handler_t entity_handler;
    struct ct_hash_t spawned_map;
    struct entity_instance *spawned_array;

    struct ct_hash_t ent_obj;
    struct ct_hash_t ent_type;
    struct ct_hash_t entity_type;
    struct type_item *entity_type_item;
};


struct simulation_pack {
    uint64_t component_mask;
    ct_simulate_fce_t simulation;
};

static struct _G {
    uint64_t type;
    uint64_t level_type;

    // NEW
    struct ct_hash_t world_map;
    struct ct_handler_t world_handler;
    struct world_instance *world_array;

    struct ct_comp_watch *comp_watch;

    struct simulation_pack *simulations;
    uint32_t component_count;

    struct ct_hash_t component_types;
    struct ct_hash_t compiler_map;
    ct_world_callbacks_t *callbacks;


    ct_component_compiler_t *compilers;

    struct ct_alloc *allocator;
} _G;


struct entity_resource {
    uint32_t ent_count;
    //uint64_t uid [ent_count]
    //uint64_t parents [ent_count]
    //uint64_t[64] ent_types[ent_count]
    //uint32_t ent_types_count[ent_count]
    //uint32_t ent_data_offset[ent_count]
    //uint32_t entity_data[ent_count]
};


#define entity_resource_uid(r) ((uint64_t*)((r) + 1))
#define entity_resource_parents(r) ((uint32_t*)(entity_resource_uid(r) + ((r)->ent_count)))
#define entity_resource_ent_types(r) ((uint64_t*)(entity_resource_parents(r) + ((r)->ent_count)))
#define entity_resource_ent_types_count(r) ((uint32_t*)(entity_resource_ent_types(r) + (64 * (r)->ent_count)))
#define entity_resource_ent_data_offset(r) ((uint32_t*)(entity_resource_ent_types_count(r) + ((r)->ent_count)))
#define entity_resource_entity_data(r) ((uint8_t*)(entity_resource_ent_data_offset(r) + ((r)->ent_count)))

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

struct world_instance *get_world_instance(struct ct_world world) {
    uint64_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT64_MAX);

    if (UINT64_MAX == idx) {
        return NULL;
    }

    return &_G.world_array[idx];
}

struct ct_cdb_obj_t *_ent_obj(struct world_instance *w,
                              struct ct_entity entity) {
    struct ct_cdb_obj_t *obj;
    obj = (struct ct_cdb_obj_t *) ct_hash_lookup(&w->ent_obj, entity.h, 0);
    return obj;
};

struct ct_cdb_obj_t *ent_obj(struct ct_world world,
                             struct ct_entity entity) {
    struct world_instance *w = get_world_instance(world);

    return _ent_obj(w, entity);
};

void register_component(uint64_t component_name) {
    uint64_t cid = ++_G.component_count;
    ct_hash_add(&_G.component_types, component_name, cid, _G.allocator);
}

uint64_t component_mask(uint64_t component_name) {
    return (uint64_t) (1
            << ct_hash_lookup(&_G.component_types, component_name, 0));
}

void _remove_from_type_slot(struct world_instance *w,
                            struct ct_entity ent,
                            uint64_t ent_type) {
    uint64_t type_idx = ct_hash_lookup(&w->entity_type,
                                       ent_type, UINT64_MAX);

    if (UINT64_MAX != type_idx) {
        struct type_item *item = &w->entity_type_item[type_idx];
        uint64_t ent_idx = ct_hash_lookup(&item->ent_map, ent.h,
                                          UINT64_MAX);

        uint32_t last_idx = ct_array_size(item->entity) - 1;
        struct ct_entity last_ent = item->entity[last_idx];

        item->entity[ent_idx] = item->entity[last_idx];
        item->objects[ent_idx] = item->objects[last_idx];

        ct_array_pop_back(item->objects);
        ct_array_pop_back(item->entity);

        ct_hash_remove(&item->ent_map, ent.h);
        ct_hash_add(&item->ent_map, last_ent.h, ent_idx, _G.allocator);
    }
}

void _add_to_type_slot(struct world_instance *w,
                       struct ct_entity ent,
                       uint64_t ent_type) {
    uint64_t type_idx = ct_hash_lookup(&w->entity_type,
                                       ent_type, UINT64_MAX);

    if (UINT64_MAX == type_idx) {
        ct_array_push(w->entity_type_item, (struct type_item) {.mask=ent_type},
                      _G.allocator);
        type_idx = ct_array_size(w->entity_type_item) - 1;
        ct_hash_add(&w->entity_type, ent_type, type_idx, _G.allocator);
    }


    struct type_item *item = &w->entity_type_item[type_idx];
    ct_array_push(item->entity, ent, _G.allocator);
    ct_array_push(item->objects, _ent_obj(w, ent), _G.allocator);

    uint32_t idx = ct_array_size(item->objects);
    ct_hash_add(&item->ent_map, ent.h, idx, _G.allocator);
}

bool has(struct ct_world world,
         struct ct_entity ent,
         uint64_t *component_name,
         uint32_t name_count) {
    struct world_instance *w = get_world_instance(world);

    uint64_t ent_type = ct_hash_lookup(&w->ent_type, ent.h, 0);

    uint64_t mask = 0;
    for (int i = 0; i < name_count; ++i) {
        mask |= (1
                << ct_hash_lookup(&_G.component_types, component_name[i], 0));
    }

    return ((ent_type & mask) == mask);
}

void add_component(struct ct_world world,
                   struct ct_entity ent,
                   uint64_t *component_name,
                   uint32_t name_count) {
    struct world_instance *w = get_world_instance(world);

    uint64_t ent_type = ct_hash_lookup(&w->ent_type, ent.h, 0);

    // REMOVE FROM PREVIOUS TYPESLOT
    if (ent_type) {
        _remove_from_type_slot(w, ent, ent_type);
    }

    uint64_t new_type = 0;
    for (int i = 0; i < name_count; ++i) {
        new_type |= (1
                << ct_hash_lookup(&_G.component_types, component_name[i], 0));
    }
    ent_type |= new_type;

    for (int j = 0; j < ct_array_size(_G.comp_watch); ++j) {
        _G.comp_watch[j].on_add(world, ent, new_type);
    }

    ct_hash_add(&w->ent_type, ent.h, ent_type, _G.allocator);

    _add_to_type_slot(w, ent, ent_type);
}

void remove_component(struct ct_world world,
                      struct ct_entity ent,
                      uint64_t component_name) {
    struct world_instance *w = get_world_instance(world);

    uint64_t ent_type = ct_hash_lookup(&w->ent_type, ent.h, 0);
    uint64_t com_type = ct_hash_lookup(&_G.component_types, component_name, 0);

    // REMOVE FROM PREVIOUS TYPESLOT
    if (ent_type) {
        _remove_from_type_slot(w, ent, ent_type);
    }

    ent_type &= ~(1 << com_type);
    ct_hash_add(&w->ent_type, ent.h, ent_type, _G.allocator);

    _add_to_type_slot(w, ent, ent_type);
}

void add_components_watch(struct ct_comp_watch watch) {
    ct_array_push(_G.comp_watch, watch, _G.allocator);
}

struct entity_instance *get_spawned_entity(struct world_instance *w, struct ct_entity ent) {
    uint64_t idx = ct_hash_lookup(&w->spawned_map, ent.h, UINT64_MAX);

    if (UINT64_MAX == idx) {
        return NULL;
    }

    return &w->spawned_array[idx];
}


void add_simulation(uint64_t components_mask,
                    ct_simulate_fce_t simulation) {
    if (!components_mask) {
        return;
    }

    struct simulation_pack sp = {
            .component_mask = components_mask,
            .simulation = simulation
    };

    ct_array_push(_G.simulations, sp, _G.allocator);
}

void simulate(struct ct_world world,
              float dt) {
    struct world_instance *w = get_world_instance(world);

    for (int i = 0; i < ct_array_size(w->entity_type_item); ++i) {
        struct type_item *item = &w->entity_type_item[i];
        for (int j = 0; j < ct_array_size(_G.simulations); ++j) {
            struct simulation_pack *sp = &_G.simulations[j];
            const uint64_t mask = sp->component_mask;
            if ((item->mask & mask) != mask) {
                continue;
            }

            sp->simulation(world,
                           item->entity, item->objects,
                           ct_array_size(item->objects), dt);
        }
    }
}

struct ct_entity create(struct ct_world world) {
    struct world_instance *w = get_world_instance(world);

    struct ct_entity ent = {.h = ct_handler_create(&w->entity_handler,
                                                   _G.allocator)};

    struct ct_cdb_obj_t *obj = ct_cdb_a0.create_object();

    ct_hash_add(&w->ent_obj, ent.h, (uint64_t) obj, _G.allocator);
    ct_hash_add(&w->ent_type, ent.h, 0, _G.allocator);

    struct ct_cdb_writer_t *wr = ct_cdb_a0.write_begin(obj);
    ct_cdb_a0.set_uint64(wr, CT_ID64_0("entid"), ent.h);
    ct_cdb_a0.write_commit(wr);

    return ent;
}

void destroy(struct ct_world world,
             struct ct_entity *entity,
             uint32_t count) {
    struct world_instance *w = get_world_instance(world);

    for (uint32_t i = 0; i < count; ++i) {
        struct entity_instance *instance = get_spawned_entity(w, entity[i]);
        if (!instance) {
            continue;
        }

        uint64_t ent_type = ct_hash_lookup(&w->ent_type, entity[i].h, 0);

        for (int j = 0; j < ct_array_size(_G.comp_watch); ++j) {
            _G.comp_watch[j].on_remove(world, entity[i], ent_type);
        }

        ct_handler_destroy(&w->entity_handler, entity[i].h, _G.allocator);

        CT_FREE(ct_memory_a0.main_allocator(), instance->entity);
    }
}

struct entity_instance *_new_entity(struct world_instance *w,
                                    uint64_t name,
                                    struct ct_entity root) {
    uint32_t idx = ct_array_size(w->spawned_array);
    ct_array_push(w->spawned_array, (struct entity_instance) {{0}},
                  _G.allocator);

    struct entity_instance *instance = &w->spawned_array[idx];
    instance->name = name;

    ct_hash_add(&w->spawned_map, root.h, idx, _G.allocator);
    return instance;
}


struct ct_entity spawn_from_resource(struct ct_world world,
                                     struct entity_instance *instance,
                                     struct entity_resource *resource,
                                     struct ct_entity *spawned) {

    struct entity_resource *res = resource;

    struct entity_instance *se = instance;
    se->world = world;
    se->entity = spawned;
    se->entity_count = res->ent_count;
    se->uid = entity_resource_uid(res);

    for (int i = 0; i < res->ent_count; ++i) {
        uint32_t offset = entity_resource_ent_data_offset(res)[i];
        uint8_t *data = entity_resource_entity_data(res);

        struct ct_cdb_obj_t *eobj = ent_obj(world, spawned[i]);
        ct_cdb_a0.load(eobj, (const char *) (data + offset), _G.allocator);

        struct ct_cdb_writer_t *w = ct_cdb_a0.write_begin(eobj);
        ct_cdb_a0.set_uint64(w, CT_ID64_0("world"), world.h);
        ct_cdb_a0.write_commit(w);

        add_component(world, spawned[i],
                      &entity_resource_ent_types(res)[i * 64],
                      entity_resource_ent_types_count(res)[i]);
    }

    return spawned[0];
}


void reload_instance(uint64_t name,
                     void *data) {
//    entity_resource *ent_res = (entity_resource *) data;
//
//    auto it = multi_map::find_first(_G.resource_map, name);
//    while (it != nullptr) {
//        struct entity_instance *instance = &_G.spawned_array[it->value];
//        //struct entity_instance old_instance = *instance;
//
//        ct_component_a0.destroy(instance->world, instance->world,
//                                instance->entity_count);
//
//        ct_entity *spawned = CT_ALLOC(ct_memory_a0.main_allocator(),
//                                          ct_entity, sizeof(ct_entity) *
//                                                     ent_res->ent_count);
//
//        uint64_t *uid = entity_resource_uid(ent_res);
//        for (uint32_t j = 0; j < ent_res->ent_count; ++j) {
//            uint32_t idx = find_by_uid(instance->uid, instance->entity_count,
//                                        uid[j]);
//            if (UINT32_MAX == idx) {
//                spawned[j] = create();
//            } else {
//                spawned[j] = instance->world[idx];
//            }
//        }
//
//        spawn_from_resource(instance->world, instance, ent_res, spawned);
//
//        it = multi_map::find_next(_G.resource_map, it);
//    }
}


//==============================================================================
// Resource
//==============================================================================

static void online(uint64_t name,
                   struct ct_vio *input,
                   struct ct_cdb_obj_t *obj) {

    const uint64_t size = input->size(input);
    char *data = CT_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

    struct ct_cdb_writer_t *writer = ct_cdb_a0.write_begin(obj);
    ct_cdb_a0.set_ptr(writer, PROP_RESOURECE_DATA, data);
    ct_cdb_a0.write_commit(writer);

    CT_UNUSED(name, data);
}

static void offline(uint64_t name,
                    struct ct_cdb_obj_t *obj) {
    CT_UNUSED(name, obj);
}

static const ct_resource_type_t callback = {
        .online = online,
        .offline = offline,
};


//==============================================================================
// Public interface
//==============================================================================
static bool alive(struct ct_world world, struct ct_entity entity) {
    struct world_instance *w = get_world_instance(world);

    return ct_handler_alive(&w->entity_handler, entity.h);
}

static struct ct_entity spawn_type(struct ct_world world,
                                   uint64_t type,
                                   uint64_t name) {

    struct ct_cdb_obj_t *obj = ct_resource_a0.get_obj(type, name);
    struct entity_resource *res = (struct entity_resource *) ct_cdb_a0.read_ptr(
            obj,
            PROP_RESOURECE_DATA,
            NULL);

    if (res == NULL) {
        ct_log_a0.error("world", "Could not spawn entity.");
        return (struct ct_entity) {.h = 0};
    }

    struct ct_entity *spawned = CT_ALLOC(_G.allocator,
                                         struct ct_entity,
                                         sizeof(struct ct_entity) *
                                         res->ent_count);

    for (uint32_t j = 0; j < res->ent_count; ++j) {
        spawned[j] = create(world);
    }

    struct world_instance *w = get_world_instance(world);
    struct entity_instance *ent_inst = _new_entity(w, name, spawned[0]);

    return spawn_from_resource(world, ent_inst, res, spawned);
}

static struct ct_entity spawn_entity(struct ct_world world,
                                     uint64_t name) {
    return spawn_type(world, _G.type, name);
}

static struct ct_entity spawn_level(struct ct_world world,
                                    uint64_t name) {
    return spawn_type(world, _G.level_type, name);
}


static struct ct_entity find_by_uid(struct ct_world world, struct ct_entity root,
                                    uint64_t uid) {
    struct world_instance *w = get_world_instance(world);
    struct entity_instance *se = get_spawned_entity(w, root);

    if (se) {
        for (int i = 0; i < se->entity_count; ++i) {
            if (se->uid[i] != uid) {
                continue;
            }

            return se->entity[i];
        }
    }

    return (struct ct_entity) {.h=0};
}

//==============================================================================
// Public interface
//==============================================================================
struct world_instance *_new_world(struct ct_world world) {
    uint32_t idx = ct_array_size(_G.world_array);
    ct_array_push(_G.world_array, (struct world_instance) {{0}}, _G.allocator);
    ct_hash_add(&_G.world_map, world.h, idx, _G.allocator);
    return &_G.world_array[idx];
}

static void register_callback(ct_world_callbacks_t clb) {
    ct_array_push(_G.callbacks, clb, _G.allocator);
}

static struct ct_world create_world() {
    struct ct_world world = {.h = ct_handler_create(&_G.world_handler,
                                                _G.allocator)};

    struct world_instance* w = _new_world(world);
    ct_handler_create(&w->entity_handler, _G.allocator);

    for (uint32_t i = 0; i < ct_array_size(_G.callbacks); ++i) {
        _G.callbacks[i].on_created(world);
    }

    return world;
}

static void destroy_world(struct ct_world world) {
    for (uint32_t i = 0; i < ct_array_size(_G.callbacks); ++i) {
        _G.callbacks[i].on_destroy(world);
    }

    struct world_instance* w = _new_world(world);
    ct_hash_free(&w->spawned_map, _G.allocator);
    ct_handler_free(&w->entity_handler, _G.allocator);
    ct_array_free(w->spawned_array, _G.allocator);

    ct_handler_destroy(&_G.world_handler, world.h, _G.allocator);
}

#include "entity_compiler.h"

static struct ct_world_a0 _api = {
        .create_entity = create,
        .ent_obj = ent_obj,
        .destroy_entity = destroy,
        .entity_alive = alive,
        .spawn_entity = spawn_entity,
        .spawn_level = spawn_level,
        .find_by_uid = find_by_uid,

        .has = has,
        .register_component = register_component,
        .component_mask = component_mask,
        .add_components = add_component,
        .remove_component = remove_component,
        .simulate = simulate,
        .add_simulation = add_simulation,
        .add_components_watch = add_components_watch,

        .register_component_compiler = register_compiler,

        .register_callback = register_callback,
        .create_world = create_world,
        .destroy_world = destroy_world,
};

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_world_a0", &_api);

}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator(),
            .type = CT_ID64_0("entity"),
            .level_type = CT_ID64_0("level"),
    };

    ct_resource_a0.register_type(_G.type, callback);
    ct_resource_a0.register_type(_G.level_type, callback);

    ct_resource_a0.compiler_register(_G.type,
                                     resource_compiler,
                                     true);
    ct_resource_a0.compiler_register(_G.level_type,
                                     resource_compiler,
                                     true);
}

static void _shutdown() {

}


CETECH_MODULE_DEF(
        world,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
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