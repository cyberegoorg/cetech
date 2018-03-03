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
#include <cetech/core/module/module.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/core/containers/hash.h>
#include <cetech/core/containers/handler.h>
#include <cetech/core/task/task.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_task_a0);

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
    struct ct_cdb_t db;

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
    struct ct_cdb_t db;
    uint32_t type;

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
    //uint8_t entity_data
};


#define entity_resource_uid(r) ((uint64_t*)((r) + 1))
#define entity_resource_parents(r) ((uint32_t*)(entity_resource_uid(r) + ((r)->ent_count)))
#define entity_resource_ent_types(r) ((uint64_t*)(entity_resource_parents(r) + ((r)->ent_count)))
#define entity_resource_ent_types_count(r) ((uint32_t*)(entity_resource_ent_types(r) + (64 * (r)->ent_count)))
#define entity_resource_ent_data_offset(r) ((uint32_t*)(entity_resource_ent_types_count(r) + ((r)->ent_count)))
#define entity_resource_prefab(r) ((uint32_t*)(entity_resource_ent_data_offset(r) + ((r)->ent_count)))
#define entity_resource_entity_data(r) ((uint8_t*)(entity_resource_prefab(r) + ((r)->ent_count)))

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
                   struct ct_cdb_obj_t *writer) {

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

void register_component(const char *component_name) {
    uint64_t cid = _G.component_count++;
    ct_hash_add(&_G.component_types, CT_ID64_0(component_name), cid,
                _G.allocator);
}

uint64_t component_mask(uint64_t component_name) {
    return (uint64_t) (1 << (uint32_t) ct_hash_lookup(&_G.component_types,
                                                      component_name, 0));
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
    uint64_t type_idx = ct_hash_lookup(&w->entity_type, ent_type, UINT64_MAX);

    if (UINT64_MAX == type_idx) {
        ct_array_push(w->entity_type_item,
                      (struct type_item) {.mask=ent_type}, _G.allocator);

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

uint64_t combine_component(uint64_t *component_name,
                           uint32_t name_count) {
    uint64_t new_type = 0;
    for (int i = 0; i < name_count; ++i) {
        new_type |= (1 << ct_hash_lookup(&_G.component_types,
                                         component_name[i], 0));
    }

    return new_type;
}

void add_components(struct ct_world world,
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
        new_type |= (1 << ct_hash_lookup(&_G.component_types,
                                         component_name[i], 0));
    }
    ent_type |= new_type;

    for (int j = 0; j < ct_array_size(_G.comp_watch); ++j) {
        _G.comp_watch[j].on_add(world, ent, new_type);
    }

    ct_hash_add(&w->ent_type, ent.h, ent_type, _G.allocator);

    _add_to_type_slot(w, ent, ent_type);
}

void remove_components(struct ct_world world,
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
        new_type |= (1 << ct_hash_lookup(&_G.component_types,
                                         component_name[i], 0));
    }

    ent_type &= ~(1 << new_type);
    ct_hash_add(&w->ent_type, ent.h, ent_type, _G.allocator);

    _add_to_type_slot(w, ent, ent_type);
}

void add_components_watch(struct ct_comp_watch watch) {
    ct_array_push(_G.comp_watch, watch, _G.allocator);
}

struct entity_instance *get_spawned_entity(struct world_instance *w,
                                           struct ct_entity ent) {
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

struct simulation_data {
    struct ct_world world;
    struct ct_entity *entities;
    struct ct_cdb_obj_t **objs;

    struct simulation_pack *sp;
    float dt;
};

void simulation_task(void *data) {
    struct simulation_data *sd = (struct simulation_data *) data;

    sd->sp->simulation(sd->world,
                       sd->entities, sd->objs, ct_array_size(sd->objs), sd->dt);
}

void simulate(struct ct_world world,
              float dt) {
    struct world_instance *w = get_world_instance(world);

    for (int j = 0; j < ct_array_size(_G.simulations); ++j) {
        struct simulation_pack *sp = &_G.simulations[j];
        const uint64_t mask = sp->component_mask;

        const uint32_t type_count = ct_array_size(w->entity_type_item);

        struct ct_task_item simulations[type_count];
        struct simulation_data simulations_data[type_count];

        uint32_t simu_n = 0;
        for (int i = 0; i < type_count; ++i) {
            struct type_item *item = &w->entity_type_item[i];

            if ((item->mask & mask) != mask) {
                continue;
            }

            uint32_t idx = simu_n++;

            simulations_data[idx] = (struct simulation_data) {
                    .world = world,
                    .objs = item->objects,
                    .entities = item->entity,
                    .dt = dt,
                    .sp = sp
            };

            simulations[idx] = (struct ct_task_item) {
                    .name="simulation",
                    .data = &simulations_data[idx],
                    .work = simulation_task};
        }

        struct ct_task_counter_t *counter;
        ct_task_a0.add(simulations, simu_n, &counter);
        ct_task_a0.wait_for_counter(counter, 0);
    }
}

void _create_entity(struct ct_world world,
                    struct ct_entity *entity,
                    uint32_t count) {
    struct world_instance *w = get_world_instance(world);

    for (int i = 0; i < count; ++i) {
        struct ct_entity ent = {.h = ct_handler_create(&w->entity_handler,
                                                       _G.allocator)};

        ct_hash_add(&w->ent_type, ent.h, 0, _G.allocator);

        entity[i] = ent;
    }

}

void create_entity(struct ct_world world,
                   struct ct_entity *entity,
                   uint32_t count) {
    struct world_instance *w = get_world_instance(world);

    for (int i = 0; i < count; ++i) {

        struct ct_entity ent = {.h = ct_handler_create(&w->entity_handler,
                                                       _G.allocator)};

        struct ct_cdb_obj_t *obj = ct_cdb_a0.create_object(w->db, 0);

        ct_hash_add(&w->ent_obj, ent.h, (uint64_t) obj, _G.allocator);
        ct_hash_add(&w->ent_type, ent.h, 0, _G.allocator);

        struct ct_cdb_obj_t *wr = ct_cdb_a0.write_begin(obj);
        ct_cdb_a0.set_uint64(wr, CT_ID64_0("entid"), ent.h);
        ct_cdb_a0.write_commit(wr);

        entity[i] = ent;
    }
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

#define PROP_ENT_OBJ (CT_ID64_0("ent_obj") << 32)


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

    struct entity_resource *res = (struct entity_resource *) data;

    struct ct_cdb_obj_t *writer = ct_cdb_a0.write_begin(obj);

    uint8_t *ent_data = entity_resource_entity_data(res);
    uint32_t *prefab = entity_resource_prefab(res);
    uint32_t *parents = entity_resource_parents(res);

    struct ct_cdb_obj_t *objs[res->ent_count];
    for (int i = 0; i < res->ent_count; ++i) {
        uint32_t offset = entity_resource_ent_data_offset(res)[i];

        struct ct_cdb_obj_t *eobj = NULL;

        if (prefab[i]) {
            struct ct_resource_id rid = (struct ct_resource_id){
                    .type = _G.type,
                    .name = prefab[i],
            };

            struct ct_cdb_obj_t *prefab_res = ct_resource_a0.get_obj(rid);
            struct ct_cdb_obj_t *prefab_obj = ct_cdb_a0.read_ref(prefab_res,
                                                                 PROP_ENT_OBJ,
                                                                 NULL);
            eobj = ct_cdb_a0.create_from(_G.db, prefab_obj);
        } else {
            eobj = ct_cdb_a0.create_object(_G.db, 0);
            ct_cdb_a0.load(eobj, (const char *) (ent_data + offset),
                           _G.allocator);
        }

        objs[i] = eobj;
        ct_cdb_a0.set_ref(writer, PROP_ENT_OBJ + i, eobj);

        uint32_t parent_idx = parents[i];
        if (parent_idx != UINT32_MAX) {
            struct ct_cdb_obj_t *parent = objs[parent_idx];
            ct_cdb_a0.add_child(parent, eobj);
        }

        struct ct_cdb_obj_t *w = ct_cdb_a0.write_begin(eobj);
        uint64_t ent_type = combine_component(
                &entity_resource_ent_types(res)[i * 64],
                entity_resource_ent_types_count(res)[i]);
        ct_cdb_a0.set_uint64(w, CT_ID64_0("ent_type"), ent_type);
        ct_cdb_a0.write_commit(w);

    }

    ct_cdb_a0.set_ptr(writer, PROP_RESOURECE_DATA, data);
    ct_cdb_a0.write_commit(writer);

    CT_UNUSED(name);
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
static bool alive(struct ct_world world,
                  struct ct_entity entity) {
    struct world_instance *w = get_world_instance(world);
    return ct_handler_alive(&w->entity_handler, entity.h);
}

static struct ct_entity spawn_entity(struct ct_world world,
                                     uint32_t name) {
    struct ct_resource_id rid = (struct ct_resource_id){
            .type = _G.type,
            .name = name,
    };

    struct ct_cdb_obj_t *obj = ct_resource_a0.get_obj(rid);
    struct entity_resource *res = (struct entity_resource *) ct_cdb_a0.read_ptr(
            obj,
            PROP_RESOURECE_DATA,
            NULL);

    if (!res) {
        ct_log_a0.error("world", "Could not spawn entity.");
        return (struct ct_entity) {.h = 0};
    }


    struct ct_entity *spawned = CT_ALLOC(_G.allocator,
                                         struct ct_entity,
                                         sizeof(struct ct_entity) *
                                         res->ent_count);

    _create_entity(world, spawned, res->ent_count);

    struct world_instance *w = get_world_instance(world);
    struct entity_instance *ent_inst = _new_entity(w, name, spawned[0]);

    struct entity_instance *se = ent_inst;
    se->world = world;
    se->entity = spawned;
    se->entity_count = res->ent_count;
    se->uid = entity_resource_uid(res);

    uint32_t *parents = entity_resource_parents(res);
    for (int i = 0; i < res->ent_count; ++i) {
        struct ct_cdb_obj_t *root = ct_cdb_a0.read_ref(obj, PROP_ENT_OBJ + i,
                                                       NULL);

        struct ct_cdb_obj_t *eobj = ct_cdb_a0.create_from(w->db, root);
        ct_hash_add(&w->ent_obj, spawned[i].h, (uint64_t) eobj, _G.allocator);

        uint32_t parent_idx = parents[i];
        if (parent_idx != UINT32_MAX) {
            struct ct_cdb_obj_t *parent = ent_obj(world, spawned[parent_idx]);
            ct_cdb_a0.add_child(parent, eobj);
        }

        struct ct_cdb_obj_t *w = ct_cdb_a0.write_begin(eobj);
        ct_cdb_a0.set_uint64(w, CT_ID64_0("world"), world.h);
        ct_cdb_a0.set_uint64(w, CT_ID64_0("entid"), spawned[i].h);
        ct_cdb_a0.write_commit(w);

        add_components(world, spawned[i],
                       &entity_resource_ent_types(res)[i * 64],
                       entity_resource_ent_types_count(res)[i]);
    }

    return spawned[0];
}

static struct ct_entity find_by_uid(struct ct_world world,
                                    struct ct_entity root,
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

    struct world_instance *w = _new_world(world);
    w->db = ct_cdb_a0.create_db();
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

    struct world_instance *w = _new_world(world);
    ct_hash_free(&w->spawned_map, _G.allocator);
    ct_handler_free(&w->entity_handler, _G.allocator);
    ct_array_free(w->spawned_array, _G.allocator);

    ct_handler_destroy(&_G.world_handler, world.h, _G.allocator);

    ct_cdb_a0.destroy_db(w->db);
}

#include "entity_compiler.h"

static struct ct_world_a0 _api = {
        .create_entity = create_entity,
        .ent_obj = ent_obj,
        .destroy_entity = destroy,
        .entity_alive = alive,
        .spawn_entity = spawn_entity,
        .find_by_uid = find_by_uid,

        .has = has,
        .register_component = register_component,
        .component_mask = component_mask,
        .add_components = add_components,
        .remove_components = remove_components,
        .simulate = simulate,
        .add_simulation = add_simulation,
        .add_components_watch = add_components_watch,

        .register_component_compiler = register_compiler,

        .register_world_callback = register_callback,
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
            .type = CT_ID32_0("entity"),
            .db = ct_cdb_a0.create_db()
    };

    ct_handler_create(&_G.world_handler, _G.allocator);

    ct_resource_a0.register_type("entity", callback);
    ct_resource_a0.compiler_register("entity", resource_compiler, true);

}

static void _shutdown() {
    ct_cdb_a0.destroy_db(_G.db);
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
            CETECH_GET_API(api, ct_task_a0);
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