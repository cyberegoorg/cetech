//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/cdb/cdb.h>
#include <cetech/engine/ecs/ecs.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/kernel/os/path.h>
#include <cetech/kernel/log/log.h>
#include <cetech/kernel/os/vio.h>
#include <cetech/kernel/hashlib/hashlib.h>
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/yaml/ydb.h>
#include <cetech/kernel/containers/hash.h>
#include <cetech/kernel/containers/handler.h>
#include <cetech/kernel/task/task.h>
#include <sys/mman.h>
#include <cetech/kernel/ebus/ebus.h>

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
CETECH_DECL_API(ct_ebus_a0);

//==============================================================================
// Globals
//==============================================================================

#define MAX_COMPONENTS 64
#define MAX_ENTITIES 100000

#define _G EntityMaagerGlobals

struct entity_storage {
    uint64_t mask;
    uint32_t n;
    struct ct_entity *entity;
    uint8_t *entity_data[MAX_COMPONENTS];
};

struct world_instance {
    struct ct_world world;
    struct ct_cdb_t db;

    // Spawn entitites
    struct ct_hash_t obj_spawn_idx_map;
    struct ct_hash_t ent_spawn_idx_map;
    struct ct_entity **obj_spawn_ent;

    // Entities
    struct ct_handler_t entity_handler;
    uint64_t *entity_type;
    uint32_t *entity_data_idx;
    uint64_t *entity_uid;

    // Hiearchy
    struct ct_entity *first_child;
    struct ct_entity *next_sibling;
    struct ct_entity *parent;

    // Storage
    struct ct_hash_t entity_storage_map;
    struct entity_storage *entity_storage;
};

static struct _G {
    struct ct_cdb_t db;
    uint32_t type;

    // WORLD
    struct ct_hash_t world_map;
    struct ct_handler_t world_handler;
    struct world_instance *world_array;

    ct_simulate_fce_t* simulations;

    uint32_t component_count;
    struct ct_hash_t component_types;

    struct ct_component_info component_info[MAX_COMPONENTS];
    struct ct_hash_t component_param_offset[MAX_COMPONENTS];
    struct ct_hash_t prop_to_comp;

    struct ct_alloc *allocator;
} _G;


static void *virtual_alloc(uint64_t size) {
    return mmap(
            NULL,
            size,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

//static void virtual_free(void* ptr, uint64_t size) {
//    munmap(ptr, size);
//}

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

static int compile(uint64_t type,
                   const char *filename,
                   uint64_t *component_key,
                   uint32_t component_key_count,
                   struct ct_cdb_obj_t *writer) {
    struct ct_ecs_component_compile_ev ev = {
            .filename = filename,
            .writer = writer,
            .component_key = component_key,
            .component_key_count = component_key_count
    };

    ct_ebus_a0.send(ECS_EBUS, ECS_COMPONENT_COMPILE, type, &ev, sizeof(ev));

    return 1;
}

static struct world_instance *get_world_instance(struct ct_world world) {
    uint64_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT64_MAX);

    if (UINT64_MAX == idx) {
        return NULL;
    }

    return &_G.world_array[idx];
}


static void register_component(struct ct_component_info info) {
    const uint64_t cid = _G.component_count++;

    const uint64_t component_hash = CT_ID64_0(info.component_name);
    info.component_name_hash = component_hash;

    _G.component_info[cid] = info;
    ct_hash_add(&_G.component_types, component_hash, cid, _G.allocator);

    for (int i = 0; i < info.prop_count; ++i) {
        const uint64_t prop_key = info.prop_map[i].key;
        const uint64_t prop_offset = info.prop_map[i].offset;

        ct_hash_add(&_G.component_param_offset[cid],
                    prop_key, prop_offset, _G.allocator);

        ct_hash_add(&_G.prop_to_comp, prop_key, component_hash, _G.allocator);
    }
}

static uint64_t component_mask(uint64_t name) {
    return (uint64_t) (1llu << ct_hash_lookup(&_G.component_types, name, 0));
}

static uint64_t component_idx(uint64_t component_name) {
    return ct_hash_lookup(&_G.component_types, component_name, UINT64_MAX);
}

static void *component_data(uint64_t component_name,
                            ct_entity_storage_t *_item) {
    struct entity_storage *item = _item;
    uint64_t com_mask = component_mask(component_name);

    if (!(com_mask & item->mask)) {
        return NULL;
    }

    uint32_t comp_idx = component_idx(component_name);
    return item->entity_data[comp_idx];

    return NULL;
}

static void *entity_data(struct ct_world world,
                         uint64_t component_name,
                         struct ct_entity entity) {
    struct world_instance *w = get_world_instance(world);
    const uint64_t ent_idx = _idx(entity.h);

    uint64_t ent_type = w->entity_type[ent_idx];

    uint64_t type_idx = ct_hash_lookup(&w->entity_storage_map, ent_type,
                                       UINT64_MAX);

    if (UINT64_MAX == type_idx) {
        return NULL;
    }

    uint64_t com_mask = component_mask(component_name);

    if (!(com_mask & ent_type)) {
        return NULL;
    }

    uint64_t com_idx = component_idx(component_name);
    struct ct_component_info info = _G.component_info[com_idx];
    struct entity_storage *item = &w->entity_storage[type_idx];
    void *comp_data = item->entity_data[com_idx];

    return comp_data + (info.size * w->entity_data_idx[ent_idx]);
}

static void entity_component_change(struct ct_world world,
                                    uint64_t component_name,
                                    struct ct_entity entity) {
    uint64_t com_mask = component_mask(component_name);

    struct ct_ecs_component_ev ev = {
            .world = world,
            .ent = entity,
            .comp_mask = com_mask,
    };

    ct_ebus_a0.send(ECS_EBUS, ECS_COMPONENT_CHANGE,
                    entity.h, &ev, sizeof(ev));
}

static void _remove_from_type_slot(struct world_instance *w,
                                   struct ct_entity ent,
                                   uint64_t ent_type) {
    uint64_t type_idx = ct_hash_lookup(&w->entity_storage_map,
                                       ent_type, UINT64_MAX);

    if (UINT64_MAX != type_idx) {
        struct entity_storage *item = &w->entity_storage[type_idx];

        uint32_t idx = w->entity_data_idx[_idx(ent.h)];
        uint32_t last_idx = item->n - 1;
        uint64_t last_ent_idx = _idx(item->entity[last_idx].h);

        w->entity_data_idx[last_ent_idx] = idx;

        for (int i = 0; i < MAX_COMPONENTS; ++i) {
            uint64_t size = _G.component_info[i].size;

            if (!item->entity_data[i]) {
                continue;
            }

            memcpy(item->entity_data[i] + (idx * size),
                   item->entity_data[i] + (last_idx * size), size);
        }

        item->n = item->n - 1;
    }
}


static void _add_to_type_slot(struct world_instance *w,
                              struct ct_entity ent,
                              uint64_t ent_type) {
    uint64_t type_idx = ct_hash_lookup(&w->entity_storage_map, ent_type,
                                       UINT64_MAX);

    if (UINT64_MAX == type_idx) {
        struct entity_storage storage = {
                .mask=ent_type,
                .n = 1,
        };
        ct_array_push(w->entity_storage, storage, _G.allocator);

        type_idx = ct_array_size(w->entity_storage) - 1;
        ct_hash_add(&w->entity_storage_map, ent_type, type_idx, _G.allocator);

        struct entity_storage *item = &w->entity_storage[type_idx];
        ct_array_push(item->entity, (struct ct_entity) {0}, _G.allocator);

        for (int i = 0; i < MAX_COMPONENTS; ++i) {
            const uint64_t component_size = _G.component_info[i].size;
            if (!component_size) {
                continue;
            }

            uint64_t mask = (1llu << i);
            if (!(ent_type & mask)) {
                continue;
            }

            item->entity_data[i] = virtual_alloc(MAX_ENTITIES * component_size);
        }
    }


    struct entity_storage *item = &w->entity_storage[type_idx];

    const uint64_t ent_idx = _idx(ent.h);
    w->entity_type[ent_idx] = ent_type;
    w->entity_data_idx[ent_idx] = item->n++;

    ct_array_push(item->entity, ent, _G.allocator);

    for (int i = 0; i < MAX_COMPONENTS; ++i) {
        uint64_t mask = (1llu << i);
        if (!(ent_type & mask)) {
            continue;
        }

        const uint64_t component_size = _G.component_info[i].size;
        memset(item->entity_data[i] +
               (component_size * w->entity_data_idx[ent_idx]), 0,
               component_size);
    }
}

static void _move_from_type_slot(struct world_instance *w,
                                 struct ct_entity ent,
                                 uint64_t ent_type,
                                 uint64_t new_type) {

    uint64_t type_idx = ct_hash_lookup(&w->entity_storage_map, ent_type,
                                       UINT64_MAX);

    uint64_t new_type_idx = ct_hash_lookup(&w->entity_storage_map, new_type,
                                           UINT64_MAX);

    struct entity_storage *item = &w->entity_storage[type_idx];
    struct entity_storage *new_item = &w->entity_storage[new_type_idx];

    const uint64_t ent_idx = _idx(ent.h);
    uint32_t idx = w->entity_data_idx[ent_idx];
    for (int i = 0; i < MAX_COMPONENTS; ++i) {
        const uint64_t component_size = _G.component_info[i].size;
        if (!component_size) {
            continue;
        }

        uint64_t mask = (1llu << i);
        if (!(ent_type & mask)) {
            continue;
        }

        if (!(new_type & mask)) {
            continue;
        }

        memcpy(new_item->entity_data[i] + (idx * component_size),
               item->entity_data[i] + (idx * component_size), component_size);
    }
}

static bool has(struct ct_world world,
                struct ct_entity ent,
                uint64_t *component_name,
                uint32_t name_count) {
    struct world_instance *w = get_world_instance(world);

    uint64_t ent_type = w->entity_type[_idx(ent.h)];

    uint64_t mask = 0;
    for (int i = 0; i < name_count; ++i) {
        mask |= (1 << ct_hash_lookup(&_G.component_types,
                                     component_name[i], 0));
    }

    return ((ent_type & mask) == mask);
}

static uint64_t combine_component(uint64_t *component_name,
                                  uint32_t name_count) {
    uint64_t new_type = 0;
    for (int i = 0; i < name_count; ++i) {
        new_type |= (1 << ct_hash_lookup(&_G.component_types,
                                         component_name[i], 0));
    }

    return new_type;
}

static void _add_components(struct ct_world world,
                            struct ct_entity ent,
                            uint64_t new_type) {
    struct world_instance *w = get_world_instance(world);

    uint64_t ent_type = w->entity_type[_idx(ent.h)];

    new_type = ent_type | new_type;

    _add_to_type_slot(w, ent, new_type);

    if (ent_type) {
        _move_from_type_slot(w, ent, ent_type, new_type);
        _remove_from_type_slot(w, ent, ent_type);
    }
}

static void add_components(struct ct_world world,
                           struct ct_entity ent,
                           uint64_t *component_name,
                           uint32_t name_count) {
    struct world_instance *w = get_world_instance(world);

    uint64_t new_type = combine_component(component_name, name_count);
    _add_components(world, ent, new_type);

    uint64_t ent_type = w->entity_type[_idx(ent.h)];

    struct ct_ecs_component_ev ev = {
            .world = world,
            .ent = ent,
            .comp_mask = ent_type,
    };

    ct_ebus_a0.broadcast(ECS_EBUS, ECS_COMPONENT_ADD, &ev, sizeof(ev));
}

static void remove_components(struct ct_world world,
                              struct ct_entity ent,
                              uint64_t *component_name,
                              uint32_t name_count) {
    struct world_instance *w = get_world_instance(world);

    uint64_t ent_type = w->entity_type[_idx(ent.h)];

    uint64_t new_type = combine_component(component_name, name_count);
    new_type &= ~(1 << new_type);

    _add_to_type_slot(w, ent, new_type);

    // REMOVE FROM PREVIOUS TYPESLOT
    if (ent_type) {
        _move_from_type_slot(w, ent, new_type, ent_type);
    }

    _remove_from_type_slot(w, ent, ent_type);
}

static void register_simulation(const char *name,
                                ct_simulate_fce_t simulation) {
    ct_array_push(_G.simulations, simulation, _G.allocator);
}

static void process(struct ct_world world,
                    uint64_t components_mask,
                    ct_process_fce_t fce,
                    void *data) {
    struct world_instance *w = get_world_instance(world);

    const uint32_t type_count = ct_array_size(w->entity_storage);

    for (int i = 0; i < type_count; ++i) {
        struct entity_storage *item = &w->entity_storage[i];

        if ((item->mask & components_mask) != components_mask) {
            continue;
        }

        fce(world, item->entity, item, item->n, data);
    }
}

static void simulate(struct ct_world world,
                     float dt) {
    for (int j = 0; j < ct_array_size(_G.simulations); ++j) {
        ct_simulate_fce_t fce = (ct_simulate_fce_t) _G.simulations[j];
        fce(world, dt);
    }
}

static void create_entities(struct ct_world world,
                            struct ct_entity *entity,
                            uint32_t count) {
    struct world_instance *w = get_world_instance(world);

    for (int i = 0; i < count; ++i) {
        struct ct_entity ent = {.h = ct_handler_create(&w->entity_handler,
                                                       _G.allocator)};

        const uint64_t idx = _idx(ent.h);
        w->entity_type[idx] = 0;
        w->entity_data_idx[idx] = 0;

        w->parent[idx] = (struct ct_entity) {0};
        w->first_child[idx] = (struct ct_entity) {0};
        w->next_sibling[idx] = (struct ct_entity) {0};

        entity[i] = ent;
    }
}


static void destroy(struct ct_world world,
                    struct ct_entity *entity,
                    uint32_t count) {
    struct world_instance *w = get_world_instance(world);


    for (uint32_t i = 0; i < count; ++i) {
        uint64_t ent_idx = _idx(entity[i].h);

        uint64_t ent_type = w->entity_type[ent_idx];
        struct ct_entity child = w->first_child[ent_idx];
        while (child.h) {
            destroy(world, &child, 1);

            child = w->next_sibling[_idx(child.h)];
        }

        uint64_t spawn_idx = ct_hash_lookup(&w->ent_spawn_idx_map, entity->h,
                                            UINT64_MAX);
        if (UINT64_MAX != spawn_idx) {
            struct ct_entity *ents = w->obj_spawn_ent[spawn_idx];
            const uint64_t size = ct_array_size(ents);
            for (int j = 0; j < size; ++j) {
                if (ents[j].h != entity[i].h) {
                    continue;
                }

                ents[j] = ents[size - 1];
                ct_array_pop_back(ents);
            }

            ct_hash_remove(&w->ent_spawn_idx_map, entity->h);
        }

        _remove_from_type_slot(w, entity[i], ent_type);

        struct ct_ecs_component_ev ev = {
                .world = world,
                .ent = entity[i],
                .comp_mask = ent_type,
        };

        ct_ebus_a0.broadcast(ECS_EBUS, ECS_COMPONENT_REMOVE, &ev, sizeof(ev));


        ct_handler_destroy(&w->entity_handler, entity[i].h, _G.allocator);
    }
}

#define PROP_ENT_OBJ (CT_ID64_0("ent_obj") << 32)

//==============================================================================
// Resource
//==============================================================================

static void _on_obj_change(uint32_t bus_name,
                           void *event) {

    struct ct_cdb_obj_change_ev *ev = event;

    for (int i = 0; i < ct_array_size(_G.world_array); ++i) {
        struct world_instance *w = &_G.world_array[i];

        uint64_t spawn_idx = ct_hash_lookup(&w->obj_spawn_idx_map,
                                            (uint64_t) ev->obj, UINT64_MAX);

        if (UINT64_MAX == spawn_idx) {
            continue;
        }

        struct ct_entity *ent = w->obj_spawn_ent[spawn_idx];
        const uint32_t ent_n = ct_array_size(ent);

        for (int k = 0; k < ev->prop_count; ++k) {
            uint64_t component = ct_hash_lookup(&_G.prop_to_comp, ev->prop[k],
                                                0);
            uint64_t comp_idx = component_idx(component);

            enum ct_cdb_type type = ct_cdb_a0.prop_type(ev->obj, ev->prop[k]);
            uint64_t prop_offset = ct_hash_lookup(
                    &_G.component_param_offset[comp_idx], ev->prop[k],
                    UINT64_MAX);

            if (prop_offset == UINT64_MAX) {
                continue;
            }


            for (int l = 0; l < ent_n; ++l) {
                uint8_t *comp_data = entity_data(w->world, component, ent[l]);

                struct data {
                    union {
                        float f;
                        float v3[3];
                        float v4[4];
                        float m16[16];
                        uint32_t u32;
                        uint64_t u64;
                        void *ptr;
                    };
                };

                struct data *dt = (struct data *) (comp_data + prop_offset);

                switch (type) {
                    case CDB_TYPE_NONE:
                    case CDB_TYPE_REF:
                    case CDB_TYPE_STR:
                        break;

                    case CDB_TYPE_UINT32:
                        dt->u32 = ct_cdb_a0.read_uint32(ev->obj, ev->prop[k],
                                                        0);
                        break;
                    case CDB_TYPE_UINT64:
                        dt->u64 = ct_cdb_a0.read_uint64(ev->obj, ev->prop[k],
                                                        0);
                        break;
                    case CDB_TYPE_PTR:
                        dt->ptr = ct_cdb_a0.read_ptr(ev->obj, ev->prop[k],
                                                     NULL);
                        break;

                    case CDB_TYPE_FLOAT:
                        dt->f = ct_cdb_a0.read_float(ev->obj, ev->prop[k],
                                                     0.0f);
                        break;

                    case CDB_TYPE_VEC3:
                        ct_cdb_a0.read_vec3(ev->obj, ev->prop[k], dt->v3);
                        break;

                    case CDB_TYPE_VEC4:
                        ct_cdb_a0.read_vec3(ev->obj, ev->prop[k], dt->v4);
                        break;

                    case CDB_TYPE_MAT4:
                        ct_cdb_a0.read_mat4(ev->obj, ev->prop[k], dt->m16);
                        break;
                }
                entity_component_change(w->world, component, ent[l]);
            }

        }
    }

}

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
            struct ct_resource_id rid = (struct ct_resource_id) {
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

        ct_ebus_a0.connect_addr(CDB_EBUS,
                                CDB_OBJ_CHANGE, (uint64_t) eobj,
                                _on_obj_change, 0);


        objs[i] = eobj;
        ct_cdb_a0.set_ref(writer, PROP_ENT_OBJ + i, eobj);

        uint32_t parent_idx = parents[i];
        if (parent_idx != UINT32_MAX) {
            struct ct_cdb_obj_t *parent = objs[parent_idx];
            ct_cdb_a0.add_child(parent, eobj);
        }

        uint64_t ent_type = combine_component(
                &entity_resource_ent_types(res)[i * 64],
                entity_resource_ent_types_count(res)[i]);
        struct ct_cdb_obj_t *w = ct_cdb_a0.write_begin(eobj);
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

static void link(struct ct_world world,
                 struct ct_entity parent,
                 struct ct_entity child) {

    struct world_instance *w = get_world_instance(world);

    w->parent[_idx(child.h)].h = parent.h;

    uint64_t tmp = w->first_child[_idx(parent.h)].h;

    w->first_child[_idx(parent.h)].h = child.h;
    w->next_sibling[_idx(child.h)].h = tmp;
}

static struct ct_entity find_by_uid(struct ct_world world,
                                    struct ct_entity root,
                                    uint64_t uid) {
    struct world_instance *w = get_world_instance(world);

    const uint64_t ent_idx = _idx(root.h);
    const uint64_t ent_uid = w->entity_uid[ent_idx];
    if (uid == ent_uid) {
        return root;
    }

    struct ct_entity child = w->first_child[ent_idx];
    while (child.h) {
        const uint64_t ent_idx = _idx(child.h);
        const uint64_t ent_uid = w->entity_uid[ent_idx];

        if (uid == ent_uid) {
            return child;
        }

        child = w->next_sibling[_idx(child.h)];
    }

    return (struct ct_entity) {.h=0};
}

static struct ct_entity spawn_entity(struct ct_world world,
                                     uint32_t name) {
    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = _G.type,
            .name = name,
    };

    struct ct_cdb_obj_t *obj = ct_resource_a0.get_obj(rid);

    struct entity_resource *res;
    res = ct_cdb_a0.read_ptr(obj, PROP_RESOURECE_DATA, NULL);

    if (!res) {
        ct_log_a0.error("entity", "Could not spawn entity.");
        return (struct ct_entity) {.h = 0};
    }

    struct ct_entity spawned[res->ent_count];
    create_entities(world, spawned, res->ent_count);

    struct world_instance *w = get_world_instance(world);

    uint32_t *parents = entity_resource_parents(res);
    for (int i = 0; i < res->ent_count; ++i) {
        struct ct_cdb_obj_t *root = ct_cdb_a0.read_ref(obj,
                                                       PROP_ENT_OBJ + i,
                                                       NULL);

        uint64_t spawn_idx = ct_hash_lookup(&w->obj_spawn_idx_map,
                                            (uint64_t) root, 0);

        if (!spawn_idx) {
            spawn_idx = ct_array_size(w->obj_spawn_ent);
            ct_array_push(w->obj_spawn_ent, 0, _G.allocator);
            ct_hash_add(&w->obj_spawn_idx_map,
                        (uint64_t) root, spawn_idx, _G.allocator);
        }

        ct_array_push(w->obj_spawn_ent[spawn_idx], spawned[i], _G.allocator);

        ct_hash_add(&w->ent_spawn_idx_map, spawned[i].h, spawn_idx,
                    _G.allocator);

        uint64_t ent_type = combine_component(
                &entity_resource_ent_types(res)[i * 64],
                entity_resource_ent_types_count(res)[i]);

        uint32_t parent_idx = parents[i];
        if (parent_idx != UINT32_MAX) {
            link(world, spawned[parent_idx], spawned[i]);
        } else {
            link(world, (struct ct_entity) {0}, spawned[i]);
        }

        _add_components(world, spawned[i], ent_type);

        const uint64_t entidx = _idx(spawned[i].h);
        w->entity_uid[entidx] = entity_resource_uid(res)[i];

        uint64_t type_idx = ct_hash_lookup(&w->entity_storage_map,
                                           ent_type, UINT64_MAX);
        struct entity_storage *item = &w->entity_storage[type_idx];
        for (int j = 0; j < MAX_COMPONENTS; ++j) {
            uint64_t mask = (1llu << j);
            if (!(ent_type & mask)) {
                continue;
            }

            const uint32_t idx = w->entity_data_idx[entidx];
            struct ct_component_info info = _G.component_info[j];

            void *comp_data = item->entity_data[j];

            struct ct_ecs_component_spawn_ev ev = {
                    .obj = root,
                    .data = comp_data + (info.size * idx),
            };

            ct_ebus_a0.send(ECS_EBUS, ECS_COMPONENT_SPAWN,
                            info.component_name_hash, &ev, sizeof(ev));
        }

        struct ct_ecs_component_ev ev = {
                .world = world,
                .ent = spawned[i],
                .comp_mask = ent_type,
        };

        ct_ebus_a0.broadcast(ECS_EBUS, ECS_COMPONENT_ADD, &ev, sizeof(ev));

    }

    return spawned[0];
}

//==============================================================================
// Public interface
//==============================================================================
static struct world_instance *_new_world(struct ct_world world) {
    uint32_t idx = ct_array_size(_G.world_array);
    ct_array_push(_G.world_array, (struct world_instance) {{0}}, _G.allocator);
    ct_hash_add(&_G.world_map, world.h, idx, _G.allocator);
    return &_G.world_array[idx];
}

static struct ct_world create_world() {
    struct ct_world world = {.h = ct_handler_create(&_G.world_handler,
                                                    _G.allocator)};

    struct world_instance *w = _new_world(world);

    w->world = world;
    w->db = ct_cdb_a0.create_db();
    ct_handler_create(&w->entity_handler, _G.allocator);

    w->entity_data_idx = virtual_alloc(sizeof(uint32_t) * MAX_ENTITIES);
    w->entity_type = virtual_alloc(sizeof(uint64_t) * MAX_ENTITIES);
    w->entity_uid = virtual_alloc(sizeof(uint64_t) * MAX_ENTITIES);
    w->parent = virtual_alloc(sizeof(uint32_t) * MAX_ENTITIES);
    w->first_child = virtual_alloc(sizeof(uint32_t) * MAX_ENTITIES);
    w->next_sibling = virtual_alloc(sizeof(uint32_t) * MAX_ENTITIES);

    struct ct_ecs_world_ev ev = {.world=world};

    ct_ebus_a0.broadcast(ECS_EBUS, ECS_WORLD_CREATE, &ev, sizeof(ev));

    return world;
}

static void destroy_world(struct ct_world world) {
    struct ct_ecs_world_ev ev = {.world=world};
    ct_ebus_a0.broadcast(ECS_EBUS, ECS_WORLD_DESTROY, &ev, sizeof(ev));

    struct world_instance *w = _new_world(world);
    ct_handler_free(&w->entity_handler, _G.allocator);
    ct_handler_destroy(&_G.world_handler, world.h, _G.allocator);

    ct_cdb_a0.destroy_db(w->db);
}

#include "entity_compiler.h"

static struct ct_ecs_a0 _api = {
        .create_entity = create_entities,
        .destroy_entity = destroy,
        .entity_alive = alive,
        .spawn_entity = spawn_entity,
        .find_by_uid = find_by_uid,
        .link = link,
        .has = has,
        .register_component = register_component,
        .component_mask = component_mask,
        .entities_data = component_data,
        .entity_data = entity_data,

        .entity_component_change = entity_component_change,
        .add_components = add_components,
        .remove_components = remove_components,
        .simulate = simulate,
        .process = process,

        .register_simulation = register_simulation,

        .create_world = create_world,
        .destroy_world = destroy_world,
};

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_ecs_a0", &_api);

}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator(),
            .type = CT_ID32_0("entity"),
            .db = ct_cdb_a0.create_db()
    };

    ct_handler_create(&_G.world_handler, _G.allocator);

    ct_ebus_a0.create_ebus(ECS_EBUS_NAME, ECS_EBUS);

    ct_resource_a0.register_type("entity", callback);
    ct_resource_a0.compiler_register("entity", resource_compiler, true);

}

static void _shutdown() {
    ct_cdb_a0.destroy_db(_G.db);
}


CETECH_MODULE_DEF(
        ecs,
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
            CETECH_GET_API(api, ct_ebus_a0);
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