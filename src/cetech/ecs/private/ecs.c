//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <sys/mman.h>

#include <celib/api_system.h>
#include <celib/memory.h>
#include <celib/cdb.h>
#include <celib/os.h>
#include <celib/log.h>
#include <celib/ebus.h>
#include <celib/hashlib.h>
#include <celib/module.h>
#include <celib/ydb.h>
#include <celib/hash.inl>
#include <celib/handler.inl>
#include <celib/task.h>

#include <cetech/ecs/ecs.h>
#include <cetech/resource/resource.h>
#include <cetech/editor/asset_preview.h>


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
    struct ce_cdb_t db;

    // Entity
    struct ce_handler_t entity_handler;

    // Storage
    struct ce_hash_t entity_storage_map;
    struct entity_storage *entity_storage;
};

static struct _G {
    struct ce_cdb_t db;

    // WORLD
    struct ce_hash_t world_map;
    struct ce_handler_t world_handler;
    struct world_instance *world_array;

    ct_simulate_fce_t *simulations;

    uint32_t component_count;
    struct ce_hash_t component_types;

    uint64_t *components_name;
    struct ce_hash_t component_interface_map;

    struct ce_alloc *allocator;
} _G;

static void *virtual_alloc(uint64_t size) {
    return mmap(NULL,
                size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

//static void virtual_free(void* ptr, uint64_t size) {
//    munmap(ptr, size);
//}



static struct ct_component_i0 *get_interface(uint64_t name) {
    return (struct ct_component_i0 *) ce_hash_lookup(
            &_G.component_interface_map, name, 0);
}

static int compile(uint64_t type,
                   const char *filename,
                   uint64_t *component_key,
                   uint32_t component_key_count,
                   ce_cdb_obj_o *writer) {

    struct ct_component_i0 *component_i = get_interface(type);

    if (!component_i) {
        ce_log_a0->error("ecs", "could not find component");
        return 0;
    }

    component_i->compiler(filename, component_key, component_key_count, writer);

    return 1;
}

#include "cetech/entity/private/entity_compiler.inl"


static struct world_instance *get_world_instance(struct ct_world world) {
    uint64_t idx = ce_hash_lookup(&_G.world_map, world.h, UINT64_MAX);

    if (UINT64_MAX == idx) {
        return NULL;
    }

    return &_G.world_array[idx];
}


const uint64_t *component_names() {
    return _G.components_name;
}

static uint64_t component_mask(uint64_t name) {
    return (uint64_t) (1llu << ce_hash_lookup(&_G.component_types, name, 0));
}

static uint64_t component_idx(uint64_t component_name) {
    return ce_hash_lookup(&_G.component_types, component_name, UINT64_MAX);
}

static void *get_all(uint64_t component_name,
                     ct_entity_storage_t *_item) {
    struct entity_storage *item = _item;
    uint64_t com_mask = component_mask(component_name);

    if (!(com_mask & item->mask)) {
        return NULL;
    }

    uint32_t comp_idx = component_idx(component_name);
    return item->entity_data[comp_idx];
}

static void *get_one(struct ct_world world,
                     uint64_t component_name,
                     struct ct_entity entity) {
    if (!entity.h) {
        return NULL;
    }

    struct world_instance *w = get_world_instance(world);
    uint64_t ent_type = ce_cdb_a0->read_uint64(entity.h, ENTITY_TYPE, 0);

    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map,
                                       ent_type, UINT64_MAX);

    if (UINT64_MAX == type_idx) {
        return NULL;
    }

    uint64_t com_mask = component_mask(component_name);

    if (!(com_mask & ent_type)) {
        return NULL;
    }

    struct ct_component_i0 *component_i = get_interface(component_name);

    uint64_t com_idx = component_idx(component_name);
    struct entity_storage *item = &w->entity_storage[type_idx];
    void *comp_data = item->entity_data[com_idx];

    uint64_t entity_data_idx;
    entity_data_idx = ce_cdb_a0->read_uint64(entity.h, ENTITY_DATA_IDX, 0);

    return comp_data + (component_i->size() * entity_data_idx);
}

static void _remove_from_type_slot(struct world_instance *w,
                                   struct ct_entity ent,
                                   uint64_t ent_type) {
    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map,
                                       ent_type, UINT64_MAX);

    if (UINT64_MAX != type_idx) {
        struct entity_storage *item = &w->entity_storage[type_idx];

        uint64_t entity_data_idx;
        entity_data_idx = ce_cdb_a0->read_uint64(ent.h, ENTITY_DATA_IDX, 0);

        uint32_t last_idx = item->n - 1;

        ce_cdb_obj_o *ent_w = ce_cdb_a0->write_begin(item->entity[last_idx].h);
        ce_cdb_a0->set_uint64(ent_w, ENTITY_DATA_IDX, entity_data_idx);
        ce_cdb_a0->write_commit(ent_w);

        const uint32_t component_n = ce_array_size(_G.components_name);
        for (int i = 0; i < component_n; ++i) {
            uint64_t component_name = _G.components_name[i];

            const uint32_t comp_idx = component_idx(component_name);

            if (!item->entity_data[comp_idx]) {
                continue;
            }

            struct ct_component_i0 *component_i = get_interface(component_name);
            uint64_t size = component_i->size();

            memcpy(item->entity_data[i] + (entity_data_idx * size),
                   item->entity_data[i] + (last_idx * size), size);
        }

        item->n = item->n - 1;
    }
}


static void _add_to_type_slot(struct world_instance *w,
                              struct ct_entity ent,
                              uint64_t ent_type) {
    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map, ent_type,
                                       UINT64_MAX);

    if (UINT64_MAX == type_idx) {
        struct entity_storage storage = {
                .mask=ent_type,
                .n = 1,
        };
        ce_array_push(w->entity_storage, storage, _G.allocator);

        type_idx = ce_array_size(w->entity_storage) - 1;
        ce_hash_add(&w->entity_storage_map, ent_type, type_idx, _G.allocator);

        struct entity_storage *item = &w->entity_storage[type_idx];
        ce_array_push(item->entity, (struct ct_entity) {0}, _G.allocator);


        const uint32_t component_n = ce_array_size(_G.components_name);
        for (int i = 0; i < component_n; ++i) {
            uint64_t component_name = _G.components_name[i];

            const uint32_t comp_idx = component_idx(component_name);

            struct ct_component_i0 *component_i = get_interface(component_name);
            uint64_t size = component_i->size();

            uint64_t mask = (1llu << comp_idx);
            if (!(ent_type & mask)) {
                continue;
            }

            item->entity_data[i] = virtual_alloc(MAX_ENTITIES * size);
        }

    }

    struct entity_storage *item = &w->entity_storage[type_idx];

    const uint64_t ent_data_idx = item->n++;

    ce_cdb_obj_o *ent_w = ce_cdb_a0->write_begin(ent.h);
    ce_cdb_a0->set_uint64(ent_w, ENTITY_TYPE, ent_type);
    ce_cdb_a0->set_uint64(ent_w, ENTITY_DATA_IDX, ent_data_idx);
    ce_cdb_a0->write_commit(ent_w);

    ce_array_push(item->entity, ent, _G.allocator);

    const uint32_t component_n = ce_array_size(_G.components_name);
    for (int i = 0; i < component_n; ++i) {
        uint64_t component_name = _G.components_name[i];

        const uint32_t comp_idx = component_idx(component_name);

        struct ct_component_i0 *component_i = get_interface(component_name);
        uint64_t size = component_i->size();

        uint64_t mask = (1llu << comp_idx);
        if (!(ent_type & mask)) {
            continue;
        }

        memset(item->entity_data[i] + (size * ent_data_idx), 0, size);
    }
}

static void _move_from_type_slot(struct world_instance *w,
                                 struct ct_entity ent,
                                 uint64_t ent_type,
                                 uint64_t new_type) {

    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map, ent_type,
                                       UINT64_MAX);

    uint64_t new_type_idx = ce_hash_lookup(&w->entity_storage_map, new_type,
                                           UINT64_MAX);

    struct entity_storage *item = &w->entity_storage[type_idx];
    struct entity_storage *new_item = &w->entity_storage[new_type_idx];

    uint32_t idx = ce_cdb_a0->read_uint64(ent.h, ENTITY_DATA_IDX, 0);

    const uint32_t component_n = ce_array_size(_G.components_name);
    for (int i = 0; i < component_n; ++i) {
        uint64_t component_name = _G.components_name[i];

        const uint32_t comp_idx = component_idx(component_name);

        struct ct_component_i0 *component_i = get_interface(component_name);
        uint64_t size = component_i->size();

        uint64_t mask = (1llu << comp_idx);
        if (!(ent_type & mask)) {
            continue;
        }

        if (!(new_type & mask)) {
            continue;
        }

        memcpy(new_item->entity_data[i] + (idx * size),
               item->entity_data[i] + (idx * size), size);
    }

}

static bool has(struct ct_world world,
                struct ct_entity ent,
                uint64_t *component_name,
                uint32_t name_count) {
    uint64_t ent_type = ce_cdb_a0->read_uint64(ent.h, ENTITY_TYPE, 0);

    uint64_t mask = 0;
    for (int i = 0; i < name_count; ++i) {
        mask |= (1 << ce_hash_lookup(&_G.component_types,
                                     component_name[i], 0));
    }

    return ((ent_type & mask) == mask);
}

static uint64_t combine_component(uint64_t *component_name,
                                  uint32_t name_count) {
    uint64_t new_type = 0;
    for (int i = 0; i < name_count; ++i) {
        new_type |= (1 << ce_hash_lookup(&_G.component_types,
                                         component_name[i], 0));
    }

    return new_type;
}

static void _add_components(struct ct_world world,
                            struct ct_entity ent,
                            uint64_t new_type) {
    struct world_instance *w = get_world_instance(world);

    uint64_t ent_type = ce_cdb_a0->read_uint64(ent.h, ENTITY_TYPE, 0);

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
    uint64_t new_type = combine_component(component_name, name_count);
    _add_components(world, ent, new_type);

}

static void remove_components(struct ct_world world,
                              struct ct_entity ent,
                              uint64_t *component_name,
                              uint32_t name_count) {
    struct world_instance *w = get_world_instance(world);

    uint64_t ent_type = ce_cdb_a0->read_uint64(ent.h, ENTITY_TYPE, 0);

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
    CE_UNUSED(name);

    ce_array_push(_G.simulations, simulation, _G.allocator);
}

static void process(struct ct_world world,
                    uint64_t components_mask,
                    ct_process_fce_t fce,
                    void *data) {
    struct world_instance *w = get_world_instance(world);

    const uint32_t type_count = ce_array_size(w->entity_storage);

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
    for (int j = 0; j < ce_array_size(_G.simulations); ++j) {
        ct_simulate_fce_t fce = (ct_simulate_fce_t) _G.simulations[j];
        fce(world, dt);
    }
}

static void create_entities(struct ct_world world,
                            struct ct_entity *entity,
                            uint32_t count) {

//    struct world_instance *w = get_world_instance(world);

    for (int i = 0; i < count; ++i) {
        uint64_t entity_obj;
        entity_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), ENTITY_INSTANCE);

        ce_cdb_obj_o *ent_w = ce_cdb_a0->write_begin(entity_obj);
        ce_cdb_a0->set_uint64(ent_w, ENTITY_TYPE, 0);
        ce_cdb_a0->set_uint64(ent_w, ENTITY_DATA_IDX, 0);
        ce_cdb_a0->set_uint64(ent_w, ENTITY_UID, 0);
        ce_cdb_a0->write_commit(ent_w);

        entity[i] = (struct ct_entity) {.h = entity_obj};

//        struct ct_entity ent = ce_handler_create(&w->entity_handler,
//                                                 _G.allocator);
    }
}


static void _destroy_with_child(struct world_instance *w,
                                uint64_t ent) {
    uint64_t childrens;
    childrens = ce_cdb_a0->read_subobject(ent, ENTITY_CHILDREN, 0);
    uint32_t children_n = ce_cdb_a0->prop_count(childrens);
    uint64_t children_keys[children_n];
    ce_cdb_a0->prop_keys(childrens, children_keys);

    uint64_t ent_type = ce_cdb_a0->read_uint64(ent, ENTITY_TYPE, 0);
    _remove_from_type_slot(w, (struct ct_entity) {.h=ent}, ent_type);

    for (int i = 0; i < children_n; ++i) {
        uint64_t children = ce_cdb_a0->read_subobject(childrens,
                                                      children_keys[i], 0);
        _destroy_with_child(w, children);
    }

//    ce_cdb_a0->destroy_object(ent);
}

static void destroy(struct ct_world world,
                    struct ct_entity *entity,
                    uint32_t count) {
    struct world_instance *w = get_world_instance(world);

    for (uint32_t i = 0; i < count; ++i) {
        _destroy_with_child(w, entity[i].h);
    }
}

static void _load(uint64_t from,
                  uint64_t parent) {

    const uint32_t prop_count = ce_cdb_a0->prop_count(from);
    uint64_t keys[prop_count];
    ce_cdb_a0->prop_keys(from, keys);

    const char *prefab = ce_cdb_a0->read_str(from, PREFAB_NAME_PROP, NULL);

    uint64_t prefab_res = 0;
    if (prefab) {
        struct ct_resource_id prefab_rid = {{{0}}};
        ct_resource_a0->type_name_from_filename(prefab, &prefab_rid, NULL);

        prefab_res = ct_resource_a0->get(prefab_rid);

        ce_cdb_a0->set_prefab(from, prefab_res);
    }

    for (int i = 0; i < prop_count; ++i) {
        uint64_t key = keys[i];
        enum ce_cdb_type type = ce_cdb_a0->prop_type(from, keys[i]);

        if (type == CDB_TYPE_SUBOBJECT) {
            uint64_t from_subobj;
            from_subobj = ce_cdb_a0->read_subobject(from, key, 0);

            uint64_t parent_subobj = 0;

            if (parent) {
                parent_subobj = ce_cdb_a0->read_subobject(parent, key, 0);
            } else if (prefab_res) {
                parent_subobj = ce_cdb_a0->read_subobject(prefab_res, key, 0);
            }

            if (parent_subobj) {
                ce_cdb_a0->set_prefab(from_subobj, parent_subobj);
            }

            _load(from_subobj, parent_subobj);
        }
    }
}

static void online(uint64_t name,
                   struct ce_vio *input,
                   uint64_t obj) {
    CE_UNUSED(name);

    const uint64_t size = input->size(input);
    char *data = CE_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

    ce_cdb_a0->load(_G.db, data, obj, _G.allocator);

    _load(obj, 0);

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(obj);
    ce_cdb_a0->set_ptr(writer, PROP_RESOURECE_DATA, data);
    ce_cdb_a0->write_commit(writer);
}

static void offline(uint64_t name,
                    uint64_t obj) {
    CE_UNUSED(name, obj);
}

static uint64_t cdb_type() {
    return ENTITY_RESOURCE_ID;
}

static struct ct_entity load(struct ct_resource_id resourceid,
                             struct ct_world world) {
    struct ct_entity ent = ct_ecs_a0->entity->spawn(world, resourceid.name);
    return ent;
}

static void unload(struct ct_resource_id resourceid,
                   struct ct_world world,
                   struct ct_entity entity) {
    ct_ecs_a0->entity->destroy(world, &entity, 1);
}


static struct ct_asset_preview_i0 ct_asset_preview_i0 = {
        .load = load,
        .unload = unload,
};

void *get_resource_interface(uint64_t name_hash) {
    if (name_hash == ASSET_PREVIEW) {
        return &ct_asset_preview_i0;
    }
    return NULL;
}

static struct ct_resource_i0 ct_resource_i0 = {
        .cdb_type = cdb_type,
        .online = online,
        .offline = offline,
        .compilator = resource_compiler,
        .get_interface = get_resource_interface,
};


//==============================================================================
// Public interface
//==============================================================================
static bool alive(struct ct_world world,
                  struct ct_entity entity) {
    return true;
}


static struct ct_entity _spawn_entity(struct ct_world world,
                                      uint64_t resource_ent) {

    uint64_t root_obj;
    root_obj = ce_cdb_a0->create_from(ce_cdb_a0->db(), resource_ent);

    struct world_instance *w = get_world_instance(world);
    ce_cdb_obj_o *wr = ce_cdb_a0->write_begin(root_obj);
    ce_cdb_a0->set_uint64(wr, ENTITY_WORLD, world.h);
    ce_cdb_a0->set_uint64(wr, ENTITY_DATA_IDX, 0);
    ce_cdb_a0->set_uint64(wr, ENTITY_UID, 0);
    ce_cdb_a0->write_commit(wr);

    uint64_t components;
    components = ce_cdb_a0->read_subobject(root_obj, ENTITY_COMPONENTS, 0);

    uint32_t components_n = ce_cdb_a0->prop_count(components);
    uint64_t components_keys[components_n];
    ce_cdb_a0->prop_keys(components, components_keys);

    uint64_t ent_type = combine_component(components_keys, components_n);

    _add_components(world, (struct ct_entity) {.h = root_obj}, ent_type);

    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map,
                                       ent_type, UINT64_MAX);

    struct entity_storage *item = &w->entity_storage[type_idx];

    const uint64_t idx = ce_cdb_a0->read_uint64(root_obj, ENTITY_DATA_IDX, 0);

    for (int i = 0; i < components_n; ++i) {
        uint64_t component_type = components_keys[i];
        uint64_t j = component_idx(component_type);

        struct ct_component_i0 *component_i;
        component_i = get_interface(component_type);

        if (!component_i) {
            continue;
        }

        void *comp_data = item->entity_data[j];

        uint64_t component_obj;
        component_obj = ce_cdb_a0->read_subobject(components,
                                                  component_type, 0);

        component_i->spawner(component_obj,
                             comp_data + (component_i->size() * idx));
    }

    uint64_t children;
    children = ce_cdb_a0->read_subobject(root_obj, ENTITY_CHILDREN, 0);
    uint32_t children_n = ce_cdb_a0->prop_count(children);
    uint64_t children_keys[children_n];
    ce_cdb_a0->prop_keys(children, children_keys);


    for (int i = 0; i < children_n; ++i) {
        uint64_t child;
        child = ce_cdb_a0->read_subobject(children, children_keys[i], 0);

        uint64_t new_obj = _spawn_entity(world, child).h;

        ce_cdb_obj_o *ch_w = ce_cdb_a0->write_begin(children);
        ce_cdb_a0->set_subobject(ch_w, children_keys[i], new_obj);
        ce_cdb_a0->write_commit(ch_w);

//        link(world, root_ent, child_ent);
    }

    return (struct ct_entity) {.h=root_obj};
}

static struct ct_entity spawn_entity(struct ct_world world,
                                     uint64_t name) {
    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = ENTITY_RESOURCE_ID,
            .name = name,
    };

    uint64_t obj = ct_resource_a0->get(rid);

    struct ct_entity root_ent = _spawn_entity(world, obj);

    return root_ent;
}

//==============================================================================
// Public interface
//==============================================================================
static struct world_instance *_new_world(struct ct_world world) {
    uint32_t idx = ce_array_size(_G.world_array);
    ce_array_push(_G.world_array, (struct world_instance) {{0}}, _G.allocator);
    ce_hash_add(&_G.world_map, world.h, idx, _G.allocator);
    return &_G.world_array[idx];
}

static struct ct_world create_world() {
    struct ct_world world = {.h = ce_handler_create(&_G.world_handler,
                                                    _G.allocator)};

    struct world_instance *w = _new_world(world);

    w->world = world;
    w->db = ce_cdb_a0->db();

    uint64_t event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                              ECS_WORLD_CREATE);

    ce_cdb_obj_o *wr = ce_cdb_a0->write_begin(event);
    ce_cdb_a0->set_uint64(wr, ENTITY_WORLD, world.h);
    ce_cdb_a0->write_commit(wr);

    ce_ebus_a0->broadcast(ECS_EBUS, event);

    return world;
}

static void destroy_world(struct ct_world world) {
    uint64_t event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                              ECS_WORLD_DESTROY);

    ce_cdb_obj_o *wr = ce_cdb_a0->write_begin(event);
    ce_cdb_a0->set_uint64(wr, ENTITY_WORLD, world.h);
    ce_cdb_a0->write_commit(wr);

    ce_ebus_a0->broadcast(ECS_EBUS, event);

    struct world_instance *w = _new_world(world);
    ce_handler_destroy(&_G.world_handler, world.h, _G.allocator);

    ce_cdb_a0->destroy_db(w->db);
}

struct ct_entity find_by_name(struct ct_world world,
                              struct ct_entity ent,
                              uint64_t name) {
    uint64_t obj = ent.h;
    uint64_t chidren = ce_cdb_a0->read_subobject(obj, ENTITY_CHILDREN, 0);
    const uint64_t children_n = ce_cdb_a0->prop_count(chidren);

    uint64_t children_key[children_n];
    ce_cdb_a0->prop_keys(chidren, children_key);

    for (int i = 0; i < children_n; ++i) {
        uint64_t child_obj = ce_cdb_a0->read_subobject(chidren, children_key[i],
                                                       0);
        const char *child_name = ce_cdb_a0->read_str(child_obj, ENTITY_NAME,
                                                     "");

        uint64_t name_hash = ce_id_a0->id64(child_name);
        if (name_hash == name) {
            return (struct ct_entity) {.h = child_obj};
        }

        struct ct_entity find_ent;
        find_ent = find_by_name(world,
                                (struct ct_entity) {.h = child_obj}, name);

        if (find_ent.h) {
            return find_ent;
        }
    }

    return (struct ct_entity) {.h = 0};
}

struct ct_entity_a0 ct_entity_a0 = {
        .create = create_entities,
        .destroy = destroy,
        .alive = alive,
        .spawn = spawn_entity,
        .has = has,
        .find_by_name = find_by_name,

        .create_world = create_world,
        .destroy_world = destroy_world,
};

struct ct_component_a0 ct_component_a0 = {
        .get_interface = get_interface,
        .mask = component_mask,
        .get_all = get_all,
        .get_one = get_one,
        .add = add_components,
        .remove = remove_components,
};

struct ct_system_a0 ct_system_a0 = {
        .register_simulation = register_simulation,
        .simulate = simulate,
        .process = process,
};


static struct ct_ecs_a0 _api = {
        .component = &ct_component_a0,
        .entity = &ct_entity_a0,
        .system = &ct_system_a0,
};

struct ct_ecs_a0 *ct_ecs_a0 = &_api;


static void _init_api(struct ce_api_a0 *api) {
    api->register_api("ct_ecs_a0", &_api);
}

static void _componet_api_add(uint64_t name,
                              void *api) {
    struct ct_component_i0 *component_i = api;

    ce_array_push(_G.components_name, component_i->cdb_type(), _G.allocator);

    ce_hash_add(&_G.component_interface_map, component_i->cdb_type(),
                (uint64_t) component_i, _G.allocator);

    const uint64_t cid = _G.component_count++;
    ce_hash_add(&_G.component_types, component_i->cdb_type(), cid,
                _G.allocator);
}

static void _init(struct ce_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .db = ce_cdb_a0->db()
    };

    ce_handler_create(&_G.world_handler, _G.allocator);

    ce_ebus_a0->create_ebus(ECS_EBUS);

    ce_api_a0->register_api(RESOURCE_I_NAME, &ct_resource_i0);
    ce_api_a0->register_on_add(COMPONENT_I, _componet_api_add);
}

static void _shutdown() {
    ce_cdb_a0->destroy_db(_G.db);
}


CE_MODULE_DEF(
        ecs,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ce_os_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_yng_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ce_task_a0);
            CE_INIT_API(api, ce_ebus_a0);
        },

        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)