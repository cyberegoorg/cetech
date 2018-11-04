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
#include <cetech/resource/sourcedb.h>


//==============================================================================
// Globals
//==============================================================================

#define MAX_COMPONENTS 64
#define MAX_ENTITIES 100000

#define _G EntityMaagerGlobals

#define LOG_WHERE "ecs"

struct entity_storage {
    uint64_t mask;
    uint32_t n;
    struct ct_entity *entity;
    uint8_t *entity_data[MAX_COMPONENTS];
};

struct spawn_info {
    struct ct_entity *ents;
};

struct world_instance {
    struct ct_world world;
    struct ce_cdb_t db;

    // Entity
    struct ce_handler_t entity_handler;
    uint64_t *entity_type;
    uint64_t *entity_idx;
    struct ct_entity *parent;
    struct ct_entity *first_child;
    struct ct_entity *next_sibling;

    // Storage
    struct ce_hash_t entity_storage_map;
    struct entity_storage *entity_storage;

    struct ce_hash_t entity_objmap;

    struct ce_hash_t component_objmap;
    struct spawn_info *component_spawn_info;

    struct ce_hash_t obj_entmap;
    struct spawn_info *obj_spawn_info;
};

#define _entity_data_idx(w, ent) \
    w->entity_idx[handler_idx((ent).h)]

#define _entity_type(w, ent) \
    w->entity_type[handler_idx((ent).h)]

struct simulation_graph {
    struct ce_hash_t graph_map;
    struct ce_hash_t fce_map;
    uint64_t **before;
    uint64_t **after;
    uint64_t *name;
    uint64_t *input_simulations;
    uint64_t *output_simulations;
};

static struct _G {
    struct ce_cdb_t db;

    // WORLD
    struct ce_hash_t world_map;
    struct ce_handler_t world_handler;
    struct world_instance *world_array;

    uint32_t component_count;
    struct ce_hash_t component_types;

    uint64_t *components_name;
    struct ce_hash_t component_interface_map;

    // SIM
    struct simulation_graph sg;

    struct ce_alloc *allocator;
} _G;

static void *virtual_alloc(uint64_t size) {
    return mmap(NULL,
                size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
}

//static void virtual_free(void* ptr, uint64_t size) {
//    munmap(ptr, size);
//}


static void _add_spawn_component_obj(struct world_instance *world,
                                     uint64_t component_obj,
                                     struct ct_entity ent) {

    uint64_t idx = ce_hash_lookup(&world->component_objmap,
                                  component_obj,
                                  UINT64_MAX);

    if (idx == UINT64_MAX) {
        idx = ce_array_size(world->component_spawn_info);
        ce_array_push(world->component_spawn_info, (struct spawn_info) {},
                      ce_memory_a0->system);

        ce_hash_add(&world->component_objmap, component_obj, idx, _G.allocator);
    }

    struct spawn_info *info = &world->component_spawn_info[idx];

    ce_array_push(info->ents, ent, ce_memory_a0->system);
}

static void _add_spawn_entity_obj(struct world_instance *world,
                                  uint64_t obj,
                                  struct ct_entity ent) {
    ce_hash_add(&world->entity_objmap, ent.h, obj, _G.allocator);

    uint64_t idx = ce_hash_lookup(&world->obj_entmap, obj, UINT64_MAX);

    if (idx == UINT64_MAX) {
        idx = ce_array_size(world->obj_spawn_info);
        ce_array_push(world->obj_spawn_info,
                      (struct spawn_info) {},
                      ce_memory_a0->system);
        ce_hash_add(&world->obj_entmap, obj, idx, _G.allocator);

    }

    struct spawn_info *info = &world->obj_spawn_info[idx];

    ce_array_push(info->ents, ent, ce_memory_a0->system);
}


static void _remove_spawn_obj(struct world_instance *world,
                              uint64_t component_obj,
                              struct ct_entity ent) {

    uint64_t idx = ce_hash_lookup(&world->component_objmap,
                                  component_obj,
                                  UINT64_MAX);

    if (idx == UINT64_MAX) {
        return;
    }

    struct spawn_info *info = &world->component_spawn_info[idx];


    const uint32_t n = ce_array_size(info->ents);
    for (int i = 0; i < n; ++i) {
        if (ent.h != info->ents[i].h) {
            continue;
        }

        info->ents[i] = info->ents[n - 1];
        ce_array_pop_back(info->ents);
        break;
    }
}

static struct ct_component_i0 *get_interface(uint64_t name) {
    return (struct ct_component_i0 *) ce_hash_lookup(
            &_G.component_interface_map, name, 0);
}

static int compile(uint64_t type,
                   const char *filename,
                   uint64_t component_key,
                   ce_cdb_obj_o *writer) {

    struct ct_component_i0 *component_i = get_interface(type);

    if (!component_i) {
        ce_log_a0->error("ecs", "could not find component");
        return 0;
    }

    component_i->compiler(filename, component_key, writer);

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

    uint64_t ent_type = _entity_type(w, entity);

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

    uint64_t entity_data_idx = _entity_data_idx(w, entity);

    return comp_data + (component_i->size() * entity_data_idx);
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
        ce_array_push(item->entity, (struct ct_entity) {}, _G.allocator);

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

            item->entity_data[comp_idx] = virtual_alloc(MAX_ENTITIES * size);
        }
    }

    struct entity_storage *item = &w->entity_storage[type_idx];

    const uint64_t ent_data_idx = item->n++;

    _entity_data_idx(w, ent) = ent_data_idx;
    _entity_type(w, ent) = ent_type;

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

        memset(item->entity_data[comp_idx] + (size * ent_data_idx), 0, size);
    }
}

static void _remove_from_type_slot(struct world_instance *w,
                                   struct ct_entity ent,
                                   uint64_t last_data_idx,
                                   uint64_t ent_type) {
    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map,
                                       ent_type, UINT64_MAX);

    if (UINT64_MAX == type_idx) {
        return;
    }

    struct entity_storage *item = &w->entity_storage[type_idx];

    uint32_t last_idx = --item->n;

    uint64_t entity_data_idx = last_data_idx;

    if (last_idx == entity_data_idx) {
        return;
    }

    struct ct_entity last_ent = item->entity[last_idx];
    _entity_data_idx(w, last_ent) = entity_data_idx;

    item->entity[entity_data_idx] = last_ent;
    ce_array_pop_back(item->entity);


    const uint32_t component_n = ce_array_size(_G.components_name);
    for (int i = 0; i < component_n; ++i) {
        uint64_t component_name = _G.components_name[i];

        const uint32_t comp_idx = component_idx(component_name);

        if (!item->entity_data[comp_idx]) {
            continue;
        }

        struct ct_component_i0 *component_i = get_interface(component_name);
        uint64_t size = component_i->size();

        memcpy(item->entity_data[comp_idx] + (entity_data_idx * size),
               item->entity_data[comp_idx] + (last_idx * size), size);
    }
}

static void _move_from_type_slot(struct world_instance *w,
                                 struct ct_entity ent,
                                 uint32_t last_idx,
                                 uint64_t ent_type,
                                 uint64_t new_type) {

    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map, ent_type,
                                       UINT64_MAX);

    uint64_t new_type_idx = ce_hash_lookup(&w->entity_storage_map, new_type,
                                           UINT64_MAX);

    struct entity_storage *item = &w->entity_storage[type_idx];
    struct entity_storage *new_item = &w->entity_storage[new_type_idx];

    uint32_t idx = _entity_data_idx(w, ent);

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

        memcpy(new_item->entity_data[comp_idx] + (idx * size),
               item->entity_data[comp_idx] + (last_idx * size), size);
    }

}

static bool has(struct ct_world world,
                struct ct_entity ent,
                uint64_t *component_name,
                uint32_t name_count) {
    struct world_instance *w = get_world_instance(world);

    uint64_t ent_type = _entity_type(w, ent);

    uint64_t mask = 0;
    for (int i = 0; i < name_count; ++i) {
        mask |= (1 << ce_hash_lookup(&_G.component_types,
                                     component_name[i], 0));
    }

    return ((ent_type & mask) == mask);
}

static uint64_t combine_component(const uint64_t *component_name,
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
                            uint64_t new_type,
                            void **data) {
    struct world_instance *w = get_world_instance(world);

    uint64_t ent_type = _entity_type(w, ent);
    uint32_t idx = _entity_data_idx(w, ent);

    new_type = ent_type | new_type;

    _add_to_type_slot(w, ent, new_type);

    if (ent_type) {
        _move_from_type_slot(w, ent, idx, ent_type, new_type);
        _remove_from_type_slot(w, ent, idx, ent_type);
    }
}

static void add_components(struct ct_world world,
                           struct ct_entity ent,
                           const uint64_t *component_name,
                           uint32_t name_count,
                           void **data) {
    uint64_t new_type = combine_component(component_name, name_count);
    _add_components(world, ent, new_type, data);

    if (data) {
        for (int i = 0; i < name_count; ++i) {
            uint64_t name = component_name[i];
            struct ct_component_i0 *ci = get_interface(name);
            void *new_comp_data = data[i];
            void *comp_data = get_one(world, name, ent);
            memcpy(comp_data, new_comp_data, ci->size());
        }
    }

}

static void remove_components(struct ct_world world,
                              struct ct_entity ent,
                              const uint64_t *component_name,
                              uint32_t name_count) {
    struct world_instance *w = get_world_instance(world);

    uint64_t ent_type = _entity_type(w, ent);
    uint64_t new_type = ent_type;
    uint64_t comp_type = combine_component(component_name, name_count);
    new_type &= ~(comp_type);

    uint32_t idx = _entity_data_idx(w, ent);

    if (new_type) {
        _add_to_type_slot(w, ent, new_type);
        _move_from_type_slot(w, ent, idx, ent_type, new_type);
    }

    _remove_from_type_slot(w, ent, idx, ent_type);
}

static const struct ct_prop_decs *desc_by_name(
        const struct ct_comp_prop_decs *desc,
        uint64_t name) {

    for (int i = 0; i < desc->prop_n; ++i) {
        if (desc->prop_decs[i].name != name) {
            continue;
        }

        return &desc->prop_decs[i];
    }

    return NULL;
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

void _sg_remove_(uint64_t *a,
                 uint64_t item) {
    uint64_t n = ce_array_size(a);
    for (int i = 0; i < n; ++i) {
        if (item != a[i]) {
            continue;
        }

        a[i] = a[n - 1];
        ce_array_pop_back(a);
        break;
    }
}

static uint64_t _sg_get_or_create(struct simulation_graph *sg,
                                  uint64_t name) {
    if (!ce_hash_contain(&sg->graph_map, name)) {
        uint64_t idx = ce_array_size(sg->after);

        ce_array_push(sg->after, NULL, _G.allocator);
        ce_array_push(sg->before, NULL, _G.allocator);
        ce_array_push(sg->name, name, _G.allocator);

        ce_hash_add(&sg->graph_map, name, idx, _G.allocator);
    }

    uint64_t idx = ce_hash_lookup(&sg->graph_map, name, UINT64_MAX);
    return idx;
}

static void _sg_clean(struct simulation_graph *sg) {
    ce_array_clean(sg->output_simulations);
    ce_array_clean(sg->input_simulations);
    ce_array_clean(sg->name);

    uint64_t n = ce_array_size(sg->after);
    for (int i = 0; i < n; ++i) {
        ce_array_clean(sg->after);
        ce_array_clean(sg->before);
    }

    ce_hash_clean(&sg->graph_map);
    ce_hash_clean(&sg->fce_map);
}

static void _sg_simulate(struct simulation_graph *sg,
                         struct ct_world world,
                         float dt) {

    const uint64_t output_n = ce_array_size(sg->output_simulations);
    for (int k = 0; k < output_n; ++k) {
        ct_simulate_fce_t fce;
        fce = (ct_simulate_fce_t) ce_hash_lookup(&sg->fce_map,
                                                 sg->output_simulations[k], 0);
        fce(world, dt);
    }

}

static void _sg_build(struct simulation_graph *sg) {
    uint64_t *input_simulations = NULL;

    _sg_clean(sg);

    struct ce_api_entry it = ce_api_a0->first(SIMULATION_INTERFACE);
    while (it.api) {
        struct ct_simulation_i0 *i = (it.api);

        uint64_t name = i->name();

        ce_hash_add(&sg->fce_map, name,
                    (uint64_t) i->simulation, _G.allocator);

        uint64_t idx = _sg_get_or_create(sg, name);

        if (i->before) {
            uint32_t before_n = 0;
            const uint64_t *before = i->before(&before_n);

            for (int b = 0; b < before_n; ++b) {
                uint64_t b_name = before[b];

                ce_array_push(sg->before[idx], b_name, _G.allocator);

                uint64_t idx = _sg_get_or_create(sg, b_name);

                ce_array_push(sg->after[idx], name, _G.allocator);
            }
        }

        if (i->after) {
            uint32_t after_n = 0;
            const uint64_t *after = i->after(&after_n);
            for (int a = 0; a < after_n; ++a) {
                uint64_t a_name = after[a];

                ce_array_push(sg->after[idx], a_name, _G.allocator);

                uint64_t idx = _sg_get_or_create(sg, a_name);

                ce_array_push(sg->before[idx], name, _G.allocator);
            }
        }

        it = ce_api_a0->next(it);
    }

    uint64_t name_n = ce_array_size(sg->name);
    for (int j = 0; j < name_n; ++j) {
        if (ce_array_size(sg->before[j])) {
            continue;
        }

        ce_array_push(input_simulations, sg->name[j], _G.allocator);
    }

    while (ce_array_size(input_simulations)) {
        uint64_t item_name = ce_array_back(input_simulations);
        ce_array_pop_back(input_simulations);

        ce_array_insert(sg->output_simulations, 0, item_name, _G.allocator);

        uint64_t dep_idx = ce_hash_lookup(&sg->graph_map, item_name,
                                          UINT64_MAX);

        uint64_t *dep_affter = sg->after[dep_idx];
        uint64_t dep_affter_n = ce_array_size(dep_affter);

        for (int i = 0; i < dep_affter_n; ++i) {
            uint64_t after_name = dep_affter[i];
            uint64_t after_idx = ce_hash_lookup(&sg->graph_map, after_name,
                                                UINT64_MAX);

            uint64_t *before = sg->before[after_idx];

            _sg_remove_(before, item_name);

            uint64_t before_n = ce_array_size(before);

            if (!before_n) {
                ce_array_push(input_simulations, after_name, _G.allocator);
            }
        }

        ce_array_clean(dep_affter);
    }
}

static void simulate(struct ct_world world,
                     float dt) {
    _sg_build(&_G.sg);
    _sg_simulate(&_G.sg, world, dt);
}

static void create_entities(struct ct_world world,
                            struct ct_entity *entity,
                            uint32_t count) {

    struct world_instance *w = get_world_instance(world);

    for (int i = 0; i < count; ++i) {
        struct ct_entity ent = {.h = ce_handler_create(&w->entity_handler,
                                                       _G.allocator)};
        entity[i] = ent;

        uint64_t idx = handler_idx(ent.h);

        w->parent[idx].h = 0;
        w->next_sibling[idx].h = 0;
        w->first_child[idx].h = 0;
    }
}

static void destroy(struct ct_world world,
                    struct ct_entity *entity,
                    uint32_t count) {
    struct world_instance *w = get_world_instance(world);

    for (uint32_t i = 0; i < count; ++i) {
        struct ct_entity ent = entity[i];

        uint64_t ent_type = _entity_type(w, ent);
        uint64_t ent_last_idx = _entity_data_idx(w, ent);
        _remove_from_type_slot(w, ent, ent_last_idx, ent_type);

        uint64_t entity_obj = ce_hash_lookup(&w->entity_objmap, ent.h, 0);
        if (entity_obj) {
            uint64_t components = ce_cdb_a0->read_subobject(entity_obj,
                                                            ENTITY_COMPONENTS,
                                                            0);

            const uint32_t components_n = ce_cdb_a0->prop_count(components);
            uint64_t components_keys[components_n];
            ce_cdb_a0->prop_keys(components, components_keys);

            for (int j = 0; j < components_n; ++j) {
                uint64_t component_obj = ce_cdb_a0->read_subobject(components,
                                                                   components_keys[j],
                                                                   0);

                if (!component_obj) {
                    continue;
                }

                _remove_spawn_obj(w, component_obj, ent);
            }

        }

        uint64_t ent_idx = handler_idx(ent.h);
        struct ct_entity ent_it = w->first_child[ent_idx];

        while (ent_it.h != 0) {
            destroy(world, &ent_it, 1);

            uint64_t idx = handler_idx(ent_it.h);
            ent_it = w->next_sibling[idx];
        }

        ce_handler_destroy(&w->entity_handler, ent.h, _G.allocator);
    }
}

static void online(uint64_t name,
                   uint64_t obj) {
    CE_UNUSED(name);
}

static void offline(uint64_t name,
                    uint64_t obj) {
    CE_UNUSED(name, obj);
}

static uint64_t cdb_type() {
    return ENTITY_RESOURCE_ID;
}

static struct ct_entity spawn_entity(struct ct_world world,
                                     uint64_t name);

static struct ct_entity load(struct ct_resource_id resourceid,
                             struct ct_world world) {
    struct ct_entity ent = spawn_entity(world, resourceid.name);
    return ent;
}

static void unload(struct ct_resource_id resourceid,
                   struct ct_world world,
                   struct ct_entity entity) {
    ct_ecs_a0->entity->destroy(world, &entity, 1);
}

void *get_resource_interface(uint64_t name_hash) {
    static struct ct_asset_preview_i0 ct_asset_preview_i0 = {
            .load = load,
            .unload = unload,
    };

    static struct ct_sourcedb_asset_i0 ct_sourcedb_asset_i0 = {
            .anotate = entity_resource_anotate,
    };

    if (name_hash == ASSET_PREVIEW) {
        return &ct_asset_preview_i0;
    } else if (name_hash == SOURCEDB_I) {
        return &ct_sourcedb_asset_i0;
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
    struct world_instance *w = get_world_instance(world);
    return ce_handler_alive(&w->entity_handler, entity.h);
}

static void _obj_to_prop(const struct ct_comp_prop_decs *comp_desc,
                         uint64_t obj,
                         uint64_t prop,
                         void *data) {
    const struct ct_prop_decs *decs = desc_by_name(comp_desc, prop);
    if (!decs) {
        return;
    }

    uint64_t offset = decs->offset;

    enum ct_ecs_prop_type prop_type = decs->type;
    switch (prop_type) {
        case ECS_PROP_FLOAT:
            *(float *) (data + offset) = ce_cdb_a0->read_float(obj,
                                                               prop,
                                                               0.0f);
            break;

        case ECS_PROP_VEC3: {
            ce_cdb_a0->read_vec3(obj, prop,
                                 ((float *) (data + offset)));
            break;
        }

        case ECS_PROP_RESOURCE_NAME:
            *(uint64_t *) (data + offset) = ce_cdb_a0->read_uint64(obj, prop,
                                                                   0);
            break;

        case ECS_PROP_STR_ID64 : {
            const char *str = ce_cdb_a0->read_str(obj, prop, "");
            *(uint64_t *) (data + offset) = ce_id_a0->id64(str);
            break;
        }

        default:
            break;
    }
}

static void _obj_change_generic(struct ct_world world,
                                struct ct_component_i0 *ci,
                                uint64_t obj,
                                const uint64_t *props,
                                uint32_t prop_count,
                                struct ct_entity *ents,
                                uint32_t n) {
    const struct ct_comp_prop_decs *comp_desc = ci->prop_desc();

    for (int i = 0; i < n; ++i) {
        struct ct_entity ent = ents[i];
        void *data = ct_ecs_a0->component->get_one(world, ci->cdb_type(), ent);

        for (int p = 0; p < prop_count; ++p) {
            uint64_t prop = props[p];
            _obj_to_prop(comp_desc, obj, prop, data);
        }
    }
}

static void _on_component_obj_change(uint64_t obj,
                                     const uint64_t *prop,
                                     uint32_t prop_count,
                                     void *data) {
    struct ct_world world = {.h= (uint64_t) data};
    struct world_instance *w = get_world_instance(world);
    uint64_t idx = ce_hash_lookup(&w->component_objmap, obj, UINT64_MAX);

    if (UINT64_MAX == idx) {
        return;
    }

    struct spawn_info *info = &w->component_spawn_info[idx];


    uint64_t component_type = ce_cdb_a0->type(obj);

    struct ct_component_i0 *component_i;
    component_i = get_interface(component_type);

    if (!component_i) {
        return;
    }


    if (!component_i->obj_change) {

        _obj_change_generic(world, component_i, obj, prop, prop_count,
                            info->ents, ce_array_size(info->ents));

    } else {
        component_i->obj_change(world,
                                obj,
                                prop,
                                prop_count,
                                info->ents,
                                ce_array_size(info->ents));
    }
}


static void _on_components_obj_add(uint64_t obj,
                                   const uint64_t *prop,
                                   uint32_t prop_count,
                                   void *data) {
    struct ct_world world = {.h= (uint64_t) data};
    struct world_instance *w = get_world_instance(world);

    uint64_t idx = ce_hash_lookup(&w->obj_entmap, ce_cdb_a0->parent(obj),
                                  UINT64_MAX);

    if (UINT64_MAX == idx) {
        return;
    }

    struct spawn_info *info = &w->obj_spawn_info[idx];

    for (int i = 0; i < prop_count; ++i) {
        uint64_t component_type = prop[i];

        uint64_t comp_obj = ce_cdb_a0->read_subobject(obj, component_type, 0);
        ce_cdb_a0->register_notify(comp_obj, _on_component_obj_change,
                                   (void *) world.h);

        const uint32_t ent_n = ce_array_size(info->ents);
        for (int j = 0; j < ent_n; ++j) {
            add_components(world, info->ents[j], &component_type, 1, NULL);
            _add_spawn_component_obj(w, comp_obj, info->ents[j]);
        }
    }
}

static void _on_components_obj_removed(uint64_t obj,
                                       const uint64_t *prop,
                                       uint32_t prop_count,
                                       void *data) {

    for (int i = 0; i < prop_count; ++i) {
        uint64_t component_obj = ce_cdb_a0->read_subobject(obj, prop[i], 0);

        struct ct_world world = {.h= (uint64_t) data};
        struct world_instance *w = get_world_instance(world);
        uint64_t idx = ce_hash_lookup(&w->component_objmap, component_obj,
                                      UINT64_MAX);

        if (UINT64_MAX == idx) {
            return;
        }

        struct spawn_info *info = &w->component_spawn_info[idx];

        const uint32_t ent_n = ce_array_size(info->ents);
        for (int j = 0; j < ent_n; ++j) {
            remove_components(world, info->ents[j], &prop[i], 1);
        }
    }
}

static void _on_entity_obj_removed(uint64_t obj,
                                   const uint64_t *prop,
                                   uint32_t prop_count,
                                   void *data) {

    for (int i = 0; i < prop_count; ++i) {
        uint64_t entity_obj = ce_cdb_a0->read_subobject(obj, prop[i], 0);

        struct ct_world world = {.h= (uint64_t) data};
        struct world_instance *w = get_world_instance(world);

        uint64_t idx = ce_hash_lookup(&w->obj_entmap, entity_obj, UINT64_MAX);

        if (idx == UINT64_MAX) {
            continue;
        }

        struct spawn_info *info = &w->obj_spawn_info[idx];

        const uint32_t ent_n = ce_array_size(info->ents);
        destroy(world, info->ents, ent_n);
    }
}


static void _on_entity_obj_add(uint64_t obj,
                               const uint64_t *prop,
                               uint32_t prop_count,
                               void *data) {

    for (int i = 0; i < prop_count; ++i) {
        uint64_t entity_obj = ce_cdb_a0->read_subobject(obj, prop[i], 0);

        struct ct_world world = {.h= (uint64_t) data};
        struct world_instance *w = get_world_instance(world);

        struct ct_entity root_ent;
        create_entities(world, &root_ent, 1);

        _add_spawn_entity_obj(w, entity_obj, root_ent);

        uint64_t components;
        components = ce_cdb_a0->read_subobject(entity_obj, ENTITY_COMPONENTS,
                                               0);

        ce_cdb_a0->register_remove_notify(components,
                                          _on_components_obj_removed,
                                          (void *) world.h);

        ce_cdb_a0->register_notify(components, _on_components_obj_add,
                                   (void *) world.h);

        uint64_t children;
        children = ce_cdb_a0->read_subobject(entity_obj, ENTITY_CHILDREN, 0);

        ce_cdb_a0->register_remove_notify(children, _on_entity_obj_removed,
                                          (void *) world.h);

        ce_cdb_a0->register_notify(children, _on_entity_obj_add,
                                   (void *) world.h);
    }
}

static void link(struct ct_world world,
                 struct ct_entity parent,
                 struct ct_entity child) {
    struct world_instance *w = get_world_instance(world);

    uint64_t child_idx = handler_idx(child.h);
    uint64_t parent_idx = handler_idx(parent.h);

    w->parent[child_idx] = parent;

    struct ct_entity tmp = w->first_child[parent_idx];

    w->first_child[parent_idx] = child;
    w->next_sibling[child_idx] = tmp;
}

static void _generic_spawner(const struct ct_component_i0 *ci,
                             uint64_t obj,
                             void *data) {
    const struct ct_comp_prop_decs *comp_desc = ci->prop_desc();

    for (int i = 0; i < comp_desc->prop_n; ++i) {
        const struct ct_prop_decs *desc = &comp_desc->prop_decs[i];
        uint64_t prop = desc->name;
        _obj_to_prop(comp_desc, obj, prop, data);
    }
}

static struct ct_entity _spawn_entity(struct ct_world world,
                                      uint64_t entity_obj) {
    struct world_instance *w = get_world_instance(world);

    struct ct_entity root_ent;
    create_entities(world, &root_ent, 1);

    uint64_t components;
    components = ce_cdb_a0->read_subobject(entity_obj, ENTITY_COMPONENTS, 0);

    uint32_t components_n = ce_cdb_a0->prop_count(components);
    uint64_t components_keys[components_n];
    ce_cdb_a0->prop_keys(components, components_keys);

    uint64_t ent_type = combine_component(components_keys, components_n);

    _add_components(world, root_ent, ent_type, NULL);

    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map,
                                       ent_type, UINT64_MAX);

    struct entity_storage *item = &w->entity_storage[type_idx];

    const uint64_t idx = _entity_data_idx(w, root_ent);

    _add_spawn_entity_obj(w, entity_obj, root_ent);

    ce_cdb_a0->register_remove_notify(components, _on_components_obj_removed,
                                      (void *) world.h);

    ce_cdb_a0->register_notify(components, _on_components_obj_add,
                               (void *) world.h);

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

        _add_spawn_component_obj(w, component_obj, root_ent);

        ce_cdb_a0->register_notify(component_obj, _on_component_obj_change,
                                   (void *) world.h);

        if (!component_i->spawner) {
            _generic_spawner(component_i, component_obj,
                             comp_data + (component_i->size() * idx));
        } else {
            component_i->spawner(world, component_obj,
                                 comp_data + (component_i->size() * idx));
        }
    }

    uint64_t children;
    children = ce_cdb_a0->read_subobject(entity_obj, ENTITY_CHILDREN, 0);

    ce_cdb_a0->register_remove_notify(children, _on_entity_obj_removed,
                                      (void *) world.h);

    ce_cdb_a0->register_notify(children, _on_entity_obj_add,
                               (void *) world.h);

    uint32_t children_n = ce_cdb_a0->prop_count(children);
    uint64_t children_keys[children_n];
    ce_cdb_a0->prop_keys(children, children_keys);
    for (int i = 0; i < children_n; ++i) {
        uint64_t child;
        child = ce_cdb_a0->read_subobject(children, children_keys[i], 0);

        struct ct_entity child_ent = _spawn_entity(world, child);

        link(world, root_ent, child_ent);
    }

    return root_ent;
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

    struct world_instance wi = {
            .entity_type = virtual_alloc(sizeof(uint64_t) * MAX_ENTITIES),
            .entity_idx  = virtual_alloc(sizeof(uint64_t) * MAX_ENTITIES),
            .parent = virtual_alloc(sizeof(struct ct_entity) * MAX_ENTITIES),
            .first_child = virtual_alloc(
                    sizeof(struct ct_entity) * MAX_ENTITIES),
            .next_sibling = virtual_alloc(
                    sizeof(struct ct_entity) * MAX_ENTITIES),
    };

    ce_array_push(_G.world_array, wi, _G.allocator);

    ce_handler_create(&_G.world_array[idx].entity_handler, _G.allocator);

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
    ce_ebus_a0->broadcast_obj(ECS_EBUS, ECS_WORLD_CREATE, event);


    return world;
}

static void destroy_world(struct ct_world world) {
    uint64_t event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                              ECS_WORLD_DESTROY);

    ce_cdb_obj_o *wr = ce_cdb_a0->write_begin(event);
    ce_cdb_a0->set_uint64(wr, ENTITY_WORLD, world.h);
    ce_cdb_a0->write_commit(wr);

    ce_ebus_a0->broadcast_obj(ECS_EBUS, ECS_WORLD_DESTROY, event);

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
        .desc_by_name = desc_by_name,
};

struct ct_system_a0 ct_system_a0 = {
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