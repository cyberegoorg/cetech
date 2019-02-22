//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <sys/mman.h>

#include <celib/api.h>
#include <celib/memory/memory.h>
#include <celib/memory/allocator.h>
#include <celib/macros.h>
#include <celib/containers/array.h>
#include <celib/containers/hash.h>
#include <celib/containers/bagraph.h>
#include <celib/cdb.h>

#include <celib/log.h>

#include <celib/id.h>
#include <celib/module.h>
#include <celib/ydb.h>
#include <celib/handler.h>
#include <celib/task.h>

#include <cetech/ecs/ecs.h>
#include <cetech/resource/resource.h>
#include <cetech/editor/resource_preview.h>
#include <cetech/kernel/kernel.h>
#include <cetech/renderer/renderer.h>
#include <celib/containers/buffer.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/transform/transform.h>
#include <stdatomic.h>
#include <cetech/editor/editor.h>
#include <cetech/game/game_system.h>


//==============================================================================
// Globals
//==============================================================================

#define MAX_COMPONENTS 64
#define MAX_ENTITIES 1000000

#define _G EntityMaagerGlobals

#define LOG_WHERE "ecs"

typedef struct entity_storage_t {
    uint64_t mask;
    uint32_t n;
    ct_entity_t0 *entity;
    uint64_t *entity_data[MAX_COMPONENTS];
} entity_storage_t;

typedef struct spawn_info_t {
    uint64_t obj;
    ct_entity_t0 *ents;
} spawn_info_t;

typedef struct world_instance_t {
    ct_world_t0 world;
    ce_cdb_t0 db;

    // Entity
    ce_handler_t0 entity_handler;
    uint64_t *entity_type;
    uint64_t *entity_idx;
    uint64_t *entity_obj;
    ct_entity_t0 *parent;
    ct_entity_t0 *first_child;
    ct_entity_t0 *next_sibling;
    ct_entity_t0 *prev_sibling;

    // Storage
    ce_hash_t entity_storage_map;
    entity_storage_t *entity_storage;

    ce_hash_t entity_objmap;

    ce_hash_t obj_entmap;
    spawn_info_t *obj_spawn_info;

    ce_hash_t comp_spawn_map;
    spawn_info_t *comp_spawn_info;

    // events
    ct_ecs_event_t0 *events;
    atomic_ullong events_n;

} world_instance_t;

#define _entity_data_idx(w, ent) \
    w->entity_idx[handler_idx((ent).h)]

#define _entity_type(w, ent) \
    w->entity_type[handler_idx((ent).h)]

#define MAX_EVENTS 100000

static struct _G {
    ce_cdb_t0 db;

    // WORLD
    ce_hash_t world_map;
    ce_handler_t0 world_handler;
    world_instance_t *world_array;

    uint32_t component_count;
    ce_hash_t component_types;

    uint64_t *components_name;
    ce_hash_t component_interface_map;

    // SIM
    ce_ba_graph_t sg;
    ce_hash_t fce_map;

    ce_alloc_t0 *allocator;
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

static void _add_event(world_instance_t *world,
                       ct_ecs_event_t0 ev) {
    uint64_t idx = atomic_fetch_add(&world->events_n, 1);
    world->events[idx] = ev;
}

static void _add_spawn_entity_obj(world_instance_t *world,
                                  uint64_t obj,
                                  struct ct_entity_t0 ent) {
    CE_ASSERT("ecs", ent.h != 0);

    ce_hash_add(&world->entity_objmap, ent.h, obj, _G.allocator);

    uint64_t idx = ce_hash_lookup(&world->obj_entmap, obj, UINT64_MAX);

    if (idx == UINT64_MAX) {
        idx = ce_array_size(world->obj_spawn_info);
        ce_array_push(world->obj_spawn_info,
                      (spawn_info_t) {.obj = obj},
                      ce_memory_a0->system);
        ce_hash_add(&world->obj_entmap, obj, idx, _G.allocator);
    }

    spawn_info_t *info = &world->obj_spawn_info[idx];

    ce_array_push(info->ents, ent, ce_memory_a0->system);
}

static void _add_spawn_comp_obj(world_instance_t *world,
                                uint64_t obj,
                                struct ct_entity_t0 ent) {
    CE_ASSERT("ecs", ent.h != 0);
    uint64_t idx = ce_hash_lookup(&world->comp_spawn_map, obj, UINT64_MAX);

    if (idx == UINT64_MAX) {
        idx = ce_array_size(world->comp_spawn_info);
        ce_array_push(world->comp_spawn_info,
                      (spawn_info_t) {.obj = obj},
                      ce_memory_a0->system);
        ce_hash_add(&world->comp_spawn_map, obj, idx, _G.allocator);
    }

    spawn_info_t *info = &world->comp_spawn_info[idx];

    ce_array_push(info->ents, ent, ce_memory_a0->system);
}

static void _remove_ent_from_spawn_info(spawn_info_t *spawn_info,
                                        struct ct_entity_t0 ent) {
    const uint32_t n = ce_array_size(spawn_info->ents);
    for (int i = 0; i < n; ++i) {
        if (spawn_info->ents[i].h != ent.h) {
            continue;
        }

        uint32_t last_idx = n - 1;
        spawn_info->ents[i] = spawn_info->ents[last_idx];
        ce_array_pop_back(spawn_info->ents);
        return;
    }
}

static void _remove_ent_from_spawn_infos(spawn_info_t *spawn_infos,
                                         struct ct_entity_t0 ent) {
    const uint32_t n = ce_array_size(spawn_infos);
    for (int i = 0; i < n; ++i) {
        _remove_ent_from_spawn_info(&spawn_infos[i], ent);
    }
}

static void _remove_spawn_info(spawn_info_t *spawn_infos,
                               struct ce_hash_t *map,
                               uint64_t obj) {
    if (!obj) {
        return;
    }

    uint64_t idx = ce_hash_lookup(map, obj, UINT64_MAX);

    if (idx == UINT64_MAX) {
        return;
    }

    ce_array_free(spawn_infos[idx].ents, _G.allocator);
    if (ce_array_size(spawn_infos)) {
        uint64_t last_idx = ce_array_size(spawn_infos) - 1;
        spawn_infos[idx] = spawn_infos[last_idx];
        ce_hash_add(map, spawn_infos[idx].obj, idx, _G.allocator);
    }

    ce_array_pop_back(spawn_infos);
    ce_hash_remove(map, obj);
}


static struct ct_component_i0 *get_interface(uint64_t name) {
    return (ct_component_i0 *) ce_hash_lookup(
            &_G.component_interface_map, name, 0);
}


static struct world_instance_t *get_world_instance(ct_world_t0 world) {
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


static uint64_t *get_all(uint64_t component_name,
                         ct_entity_storage_o0 *_item) {
    entity_storage_t *item = (entity_storage_t *) _item;
    uint64_t com_mask = component_mask(component_name);

    if (!(com_mask & item->mask)) {
        return NULL;
    }

    uint32_t comp_idx = component_idx(component_name);
    return item->entity_data[comp_idx] + 1;
}

static uint64_t *_get_one(ct_world_t0 world,
                          uint64_t component_name,
                          struct ct_entity_t0 entity) {
    if (!entity.h) {
        return 0;
    }

    world_instance_t *w = get_world_instance(world);

    uint64_t ent_type = _entity_type(w, entity);

    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map,
                                       ent_type, UINT64_MAX);

    if (UINT64_MAX == type_idx) {
        return 0;
    }

    ct_component_i0 *c = get_interface(component_name);

    if (!c) {
        return 0;
    }

    uint64_t com_mask = component_mask(component_name);

    if (!(com_mask & ent_type)) {
        return 0;
    }

    uint64_t com_idx = component_idx(component_name);
    entity_storage_t *item = &w->entity_storage[type_idx];
    uint64_t *comp_data = item->entity_data[com_idx];

    uint64_t entity_data_idx = _entity_data_idx(w, entity);
    return &comp_data[entity_data_idx];
}

static uint64_t get_one(ct_world_t0 world,
                        uint64_t component_name,
                        ct_entity_t0 entity) {
    uint64_t *c = _get_one(world, component_name, entity);
    if (c) {
        return *c;
    } else {
        return 0;
    }
}

static void _add_to_type_slot(world_instance_t *w,
                              struct ct_entity_t0 ent,
                              uint64_t ent_type) {
    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map, ent_type, UINT64_MAX);

    if (UINT64_MAX == type_idx) {
        struct entity_storage_t storage = {
                .mask=ent_type,
                .n = 1,
        };
        ce_array_push(w->entity_storage, storage, _G.allocator);

        type_idx = ce_array_size(w->entity_storage) - 1;
        ce_hash_add(&w->entity_storage_map, ent_type, type_idx, _G.allocator);

        struct entity_storage_t *item = &w->entity_storage[type_idx];
        item->entity = virtual_alloc(MAX_ENTITIES * sizeof(ct_entity_t0));

        const uint32_t component_n = ce_array_size(_G.components_name);
        for (int i = 0; i < component_n; ++i) {
            uint64_t component_name = _G.components_name[i];

            const uint32_t comp_idx = component_idx(component_name);

            uint64_t mask = (1llu << comp_idx);
            if (!(ent_type & mask)) {
                continue;
            }

            item->entity_data[comp_idx] = virtual_alloc(MAX_ENTITIES * sizeof(uint64_t));
        }
    }

    entity_storage_t *item = &w->entity_storage[type_idx];

    const uint64_t ent_data_idx = item->n++;

    _entity_data_idx(w, ent) = ent_data_idx;
    _entity_type(w, ent) = ent_type;

    item->entity[ent_data_idx] = ent;

    const uint32_t component_n = ce_array_size(_G.components_name);
    for (int i = 0; i < component_n; ++i) {
        uint64_t component_name = _G.components_name[i];

        const uint32_t comp_idx = component_idx(component_name);

        uint64_t mask = (1llu << comp_idx);
        if (!(ent_type & mask)) {
            continue;
        }

        item->entity_data[comp_idx][ent_data_idx] = 0;
    }
}

static void _remove_from_type_slot(world_instance_t *w,
                                   struct ct_entity_t0 ent,
                                   uint64_t ent_idx,
                                   uint64_t ent_type) {
    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map, ent_type, UINT64_MAX);

    if (UINT64_MAX == type_idx) {
        return;
    }

    entity_storage_t *item = &w->entity_storage[type_idx];

    if (item->n <= 1) {
        return;
    }

    uint32_t last_idx = --item->n;

    uint64_t entity_data_idx = ent_idx;

    if (last_idx == entity_data_idx) {
        return;
    }

    ct_entity_t0 last_ent = item->entity[last_idx];
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

        item->entity_data[comp_idx][entity_data_idx] = item->entity_data[comp_idx][last_idx];
    }
}

static void _move_data_from_type_slot(world_instance_t *w,
                                      struct ct_entity_t0 ent,
                                      uint32_t old_idx,
                                      uint64_t ent_type,
                                      uint64_t new_type) {

    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map, ent_type,
                                       UINT64_MAX);

    uint64_t new_type_idx = ce_hash_lookup(&w->entity_storage_map, new_type,
                                           UINT64_MAX);

    entity_storage_t *item = &w->entity_storage[type_idx];
    entity_storage_t *new_item = &w->entity_storage[new_type_idx];

    uint32_t idx = _entity_data_idx(w, ent);

    const uint32_t component_n = ce_array_size(_G.components_name);
    for (int i = 0; i < component_n; ++i) {
        uint64_t component_name = _G.components_name[i];

        const uint32_t comp_idx = component_idx(component_name);

        uint64_t mask = (1llu << comp_idx);
        if (!(ent_type & mask)) {
            continue;
        }

        if (!(new_type & mask)) {
            continue;
        }

        new_item->entity_data[comp_idx][idx] = item->entity_data[comp_idx][old_idx];
    }

}

static bool has(ct_world_t0 world,
                struct ct_entity_t0 ent,
                uint64_t *component_name,
                uint32_t name_count) {
    world_instance_t *w = get_world_instance(world);

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

static void _add_components(ct_world_t0 world,
                            struct ct_entity_t0 ent,
                            uint64_t new_type) {
    world_instance_t *w = get_world_instance(world);

    uint64_t ent_type = _entity_type(w, ent);
    uint32_t idx = _entity_data_idx(w, ent);

    new_type = ent_type | new_type;

    if (ent_type == new_type) {
        return;
    }

    _add_to_type_slot(w, ent, new_type);

    if (ent_type) {
        _move_data_from_type_slot(w, ent, idx, ent_type, new_type);
        _remove_from_type_slot(w, ent, idx, ent_type);
    }
}

static void add_components(ct_world_t0 world,
                           struct ct_entity_t0 ent,
                           const uint64_t *components,
                           uint32_t components_count) {
    uint64_t types[components_count];
    for (int i = 0; i < components_count; ++i) {
        types[i] = ce_cdb_a0->obj_type(ce_cdb_a0->db(), components[i]);
    }
    uint64_t new_type = combine_component(types, components_count);

    _add_components(world, ent, new_type);

    world_instance_t *w = get_world_instance(world);
    for (int i = 0; i < components_count; ++i) {
        uint64_t obj = components[i];

        _add_event(w, (ct_ecs_event_t0) {
                .type = CT_ECS_EVENT_COMPONENT_SPAWN,
                .component = {
                        .ent = ent,
                        .component = obj,
                }
        });

        uint64_t *comp_data = _get_one(world, types[i], ent);
        *comp_data = obj;
    }
}

static void _add_components_from_obj(world_instance_t *world,
                                     struct ct_entity_t0 ent,
                                     const uint64_t component_name,
                                     uint64_t obj) {
    uint64_t mew_component = ce_cdb_a0->create_from(ce_cdb_a0->db(), obj);
    add_components(world->world, ent, &mew_component, 1);
    _add_spawn_comp_obj(world, obj, ent);
}

static void remove_components(ct_world_t0 world,
                              struct ct_entity_t0 ent,
                              const uint64_t *component_name,
                              uint32_t name_count) {
    world_instance_t *w = get_world_instance(world);

    uint64_t ent_type = _entity_type(w, ent);
    uint64_t new_type = ent_type;
    uint64_t comp_type = combine_component(component_name, name_count);
    new_type &= ~(comp_type);

    uint32_t idx = _entity_data_idx(w, ent);

    for (int i = 0; i < name_count; ++i) {
        uint64_t component = get_one(world, component_name[i], ent);
        ce_cdb_a0->destroy_object(ce_cdb_a0->db(), component);
    }


    if (new_type) {
        _add_to_type_slot(w, ent, new_type);
        _move_data_from_type_slot(w, ent, idx, ent_type, new_type);
    }

    _remove_from_type_slot(w, ent, idx, ent_type);

    if (!new_type) {
        _entity_type(w, ent) = 0;
    }

}

static void process(ct_world_t0 world,
                    uint64_t components_mask,
                    ct_process_fce_t fce,
                    void *data) {
    world_instance_t *w = get_world_instance(world);

    const uint32_t type_count = ce_array_size(w->entity_storage);

    for (int i = 0; i < type_count; ++i) {
        struct entity_storage_t *item = &w->entity_storage[i];

        if (!item->n) {
            continue;
        }

        if ((item->mask & components_mask) != components_mask) {
            continue;
        }

        fce(world, item->entity + 1, (ct_entity_storage_o0 *) item, item->n - 1, data);
    }
}

static void _simulate_world(ce_ba_graph_t *sg,
                            ct_world_t0 world,
                            float dt) {

    const uint64_t output_n = ce_array_size(sg->output);
    for (int k = 0; k < output_n; ++k) {
        ct_simulate_fce_t fce;
        fce = (ct_simulate_fce_t) ce_hash_lookup(&_G.fce_map,
                                                 sg->output[k], 0);
        fce(world, dt);
    }

}

static void _build_sim_graph(ce_ba_graph_t *sg) {
    ce_bag_clean(sg);
    ce_hash_clean(&_G.fce_map);

    ce_api_entry_t0 it = ce_api_a0->first(SIMULATION_INTERFACE);
    while (it.api) {
        struct ct_simulation_i0 *i = (it.api);

        uint64_t name = i->name();

        ce_hash_add(&_G.fce_map, name,
                    (uint64_t) i->simulation, _G.allocator);


        uint32_t before_n = 0;
        const uint64_t *before = NULL;
        if (i->before) {
            before = i->before(&before_n);
        }

        uint32_t after_n = 0;
        const uint64_t *after;
        if (i->after) {
            after = i->after(&after_n);
        }

        ce_bag_add(&_G.sg, name,
                   before, before_n,
                   after, after_n, _G.allocator);

        it = ce_api_a0->next(it);
    }

    ce_bag_build(&_G.sg, _G.allocator);
}

static void simulate(ct_world_t0 world,
                     float dt) {
    _build_sim_graph(&_G.sg);
    _simulate_world(&_G.sg, world, dt);
}

static void create_entities(ct_world_t0 world,
                            struct ct_entity_t0 *entity,
                            uint32_t count) {

    world_instance_t *w = get_world_instance(world);

    for (int i = 0; i < count; ++i) {
        struct ct_entity_t0 ent = {.h = ce_handler_create(&w->entity_handler,
                                                          _G.allocator)};

        CE_ASSERT("ecs", ent.h != 0);

        entity[i] = ent;

        uint64_t idx = handler_idx(ent.h);

        w->parent[idx].h = 0;
        w->next_sibling[idx].h = 0;
        w->prev_sibling[idx].h = 0;
        w->first_child[idx].h = 0;

        w->entity_obj[idx] = 0;
    }
}

static void create_entities_objs(ct_world_t0 world,
                                 struct ct_entity_t0 *entity,
                                 uint32_t count,
                                 uint64_t *objs) {

    world_instance_t *w = get_world_instance(world);

    for (int i = 0; i < count; ++i) {
        struct ct_entity_t0 ent = {.h = ce_handler_create(&w->entity_handler,
                                                          _G.allocator)};

        CE_ASSERT("ecs", ent.h != 0);

        entity[i] = ent;

        uint64_t idx = handler_idx(ent.h);

        w->parent[idx].h = 0;
        w->next_sibling[idx].h = 0;
        w->prev_sibling[idx].h = 0;
        w->first_child[idx].h = 0;

        uint64_t obj = 0;
        if (objs[i]) {
            obj = objs[i];
        }

        w->entity_obj[idx] = obj;
    }
}

void unlink(ct_world_t0 world,
            struct ct_entity_t0 ent) {
    world_instance_t *w = get_world_instance(world);

    uint64_t ent_idx = handler_idx(ent.h);

    ct_entity_t0 parent = w->parent[ent_idx];
    if (!parent.h) {
        return;
    }

    uint64_t parent_idx = handler_idx(ent.h);

    ct_entity_t0 prev_ent = w->prev_sibling[ent_idx];
    uint64_t prev_ent_idx = handler_idx(prev_ent.h);

    ct_entity_t0 next_ent = w->next_sibling[ent_idx];
    uint64_t nex_ent_idx = handler_idx(next_ent.h);

    w->prev_sibling[ent_idx].h = 0;
    w->next_sibling[ent_idx].h = 0;

    // first in root
    if (!prev_ent.h) {
        w->first_child[parent_idx] = next_ent;
        w->prev_sibling[nex_ent_idx].h = 0;
        return;
    }

    if (next_ent.h) {
        w->next_sibling[prev_ent_idx] = next_ent;
        w->prev_sibling[nex_ent_idx] = prev_ent;
        return;
    }

    _add_event(w, (ct_ecs_event_t0) {
            .link.child = ent,
            .type=CT_ECS_EVENT_ENT_UNLINK,
    });
}

static void destroy(ct_world_t0 world,
                    struct ct_entity_t0 *entity,
                    uint32_t count) {
    world_instance_t *w = get_world_instance(world);

    for (uint32_t i = 0; i < count; ++i) {
        struct ct_entity_t0 ent = entity[i];

        uint64_t ent_type = _entity_type(w, ent);
        uint64_t ent_last_idx = _entity_data_idx(w, ent);

        if (ent_type) {
            uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map, ent_type, UINT64_MAX);
            uint64_t **ent_data = w->entity_storage[type_idx].entity_data;
            for (int j = 1; j < MAX_COMPONENTS; ++j) {
                if (!ent_data[j]) {
                    continue;
                }

                uint64_t obj = ent_data[j][ent_last_idx];
                if (obj) {
                    ce_cdb_a0->destroy_object(ce_cdb_a0->db(), obj);
                }
            }
        }

        _remove_from_type_slot(w, ent, ent_last_idx, ent_type);

        _entity_type(w, ent) = 0;

        uint64_t ent_idx = handler_idx(ent.h);
        struct ct_entity_t0 ent_it = w->first_child[ent_idx];


        struct ct_entity_t0 *ent_to_dest = NULL;
        while (ent_it.h != 0) {
            uint64_t idx = handler_idx(ent_it.h);

            ce_array_push(ent_to_dest, ent_it, _G.allocator);

            ent_it = w->next_sibling[idx];
        }

        destroy(world, ent_to_dest, ce_array_size(ent_to_dest));
        ce_array_free(ent_to_dest, _G.allocator);

        _remove_ent_from_spawn_infos(w->comp_spawn_info, ent);
        _remove_ent_from_spawn_infos(w->obj_spawn_info, ent);

        unlink(world, ent);
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

static struct ct_entity_t0 spawn_entity(ct_world_t0 world,
                                        uint64_t name);

static struct ct_entity_t0 load(uint64_t resource,
                                ct_world_t0 world) {

    ct_entity_t0 ent = spawn_entity(world, resource);

    return ent;
}


void *get_resource_interface(uint64_t name_hash) {
    static struct ct_resource_preview_i0 ct_resource_preview_i0 = {
            .load = load,
    };

    if (name_hash == RESOURCE_PREVIEW_I) {
        return &ct_resource_preview_i0;
    };

    return NULL;
}

void create_new(uint64_t obj) {
    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);

    if (!ce_cdb_a0->prop_exist(w, ENTITY_CHILDREN)) {
        uint64_t ch = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                               ENTITY_CHILDREN);
        ce_cdb_a0->set_subobject(w, ENTITY_CHILDREN, ch);
    }

    if (!ce_cdb_a0->prop_exist(w, ENTITY_COMPONENTS)) {
        uint64_t ch = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                               ENTITY_COMPONENTS);
        ce_cdb_a0->set_subobject(w, ENTITY_COMPONENTS, ch);
    }

    ce_cdb_a0->write_commit(w);
}

static const char *display_icon() {
    return ICON_FA_CUBES;
}

static struct ct_resource_i0 ct_resource_api = {
        .cdb_type = cdb_type,
        .display_icon = display_icon,
        .online = online,
        .offline = offline,
        .get_interface = get_resource_interface,
//        .create_new = create_new,
};

//==============================================================================
// Public interface
//==============================================================================

static bool alive(ct_world_t0 world,
                  struct ct_entity_t0 entity) {
    world_instance_t *w = get_world_instance(world);
    return ce_handler_alive(&w->entity_handler, entity.h);
}

static void link(ct_world_t0 world,
                 struct ct_entity_t0 parent,
                 struct ct_entity_t0 child) {
    world_instance_t *w = get_world_instance(world);

    uint64_t child_idx = handler_idx(child.h);
    uint64_t parent_idx = handler_idx(parent.h);

    w->parent[child_idx] = parent;

    ct_entity_t0 tmp = w->first_child[parent_idx];
    uint64_t tmp_idx = handler_idx(tmp.h);

    w->first_child[parent_idx] = child;
    w->next_sibling[child_idx] = tmp;
    w->prev_sibling[child_idx].h = 0;

    if (tmp.h) {
        w->prev_sibling[tmp_idx] = child;
    }

    _add_event(w, (ct_ecs_event_t0) {
            .link.child = child,
            .link.parent= parent,
            .type=CT_ECS_EVENT_ENT_LINK,
    });
}

static struct ct_entity_t0 parent(ct_world_t0 world,
                                  struct ct_entity_t0 entity) {
    world_instance_t *w = get_world_instance(world);

    uint64_t ent_idx = handler_idx(entity.h);

    return w->parent[ent_idx];
}

static struct ct_entity_t0 first_child(ct_world_t0 world,
                                       struct ct_entity_t0 entity) {
    world_instance_t *w = get_world_instance(world);

    uint64_t ent_idx = handler_idx(entity.h);

    return w->first_child[ent_idx];
}

static struct ct_entity_t0 next_sibling(ct_world_t0 world,
                                        struct ct_entity_t0 entity) {
    world_instance_t *w = get_world_instance(world);

    uint64_t ent_idx = handler_idx(entity.h);

    return w->next_sibling[ent_idx];
}

static struct ct_entity_t0 spawn_entity(ct_world_t0 world,
                                        uint64_t name) {

    world_instance_t *w = get_world_instance(world);

    ct_entity_t0 root_ent;
    create_entities_objs(world, &root_ent, 1, &name);


    const ce_cdb_obj_o0 *ent_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                      name);

    uint64_t components;
    components = ce_cdb_a0->read_subobject(ent_reader, ENTITY_COMPONENTS, 0);

    const ce_cdb_obj_o0 *comp_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                       components);

    uint32_t components_n = ce_cdb_a0->prop_count(comp_reader);
    const uint64_t *components_keys = ce_cdb_a0->prop_keys(comp_reader);

    uint64_t ent_type = combine_component(components_keys, components_n);

    _add_components(world, root_ent, ent_type);

    _add_spawn_entity_obj(w, name, root_ent);

    uint64_t type_idx = ce_hash_lookup(&w->entity_storage_map,
                                       ent_type, UINT64_MAX);

    entity_storage_t *item = &w->entity_storage[type_idx];

    const uint64_t idx = _entity_data_idx(w, root_ent);

    for (int i = 0; i < components_n; ++i) {
        uint64_t component_type = components_keys[i];
        uint64_t j = component_idx(component_type);

        struct ct_component_i0 *component_i;
        component_i = get_interface(component_type);

        if (!component_i) {
            continue;
        }

        uint64_t component_obj;
        component_obj = ce_cdb_a0->read_subobject(comp_reader, component_type, 0);

        _add_spawn_comp_obj(w, component_obj, root_ent);

//        uint64_t new_comp_obj = component_obj;
        uint64_t new_comp_obj = ce_cdb_a0->create_from(ce_cdb_a0->db(), component_obj);
        uint64_t *comp_data = item->entity_data[j];
        comp_data[idx] = new_comp_obj;

        _add_event(w, (ct_ecs_event_t0) {
                .type = CT_ECS_EVENT_COMPONENT_SPAWN,
                .component = {
                        .ent = root_ent,
                        .component = new_comp_obj
                }
        });
    }

    uint64_t children;
    children = ce_cdb_a0->read_subobject(ent_reader, ENTITY_CHILDREN, 0);

    const ce_cdb_obj_o0 *ch_reader = ce_cdb_a0->read(ce_cdb_a0->db(), children);

    uint32_t children_n = ce_cdb_a0->prop_count(ch_reader);
    const uint64_t *children_keys = ce_cdb_a0->prop_keys(ch_reader);
    for (int i = 0; i < children_n; ++i) {
        uint64_t child;
        child = ce_cdb_a0->read_subobject(ch_reader, children_keys[i], 0);

        struct ct_entity_t0 child_ent = spawn_entity(world, child);

        link(world, root_ent, child_ent);
    }

    return root_ent;
}

//==============================================================================
// Public interface
//==============================================================================
static struct world_instance_t *_new_world(ct_world_t0 world) {
    uint32_t idx = ce_array_size(_G.world_array);

    world_instance_t wi = {
            .entity_type = virtual_alloc(sizeof(uint64_t) * MAX_ENTITIES),
            .entity_idx  = virtual_alloc(sizeof(uint64_t) * MAX_ENTITIES),
            .entity_obj = virtual_alloc(sizeof(uint64_t) * MAX_ENTITIES),

            .parent = virtual_alloc(sizeof(ct_entity_t0) * MAX_ENTITIES),
            .first_child = virtual_alloc(sizeof(ct_entity_t0) * MAX_ENTITIES),
            .next_sibling = virtual_alloc(sizeof(ct_entity_t0) * MAX_ENTITIES),
            .prev_sibling = virtual_alloc(sizeof(ct_entity_t0) * MAX_ENTITIES),

            .events = virtual_alloc(MAX_EVENTS * sizeof(ct_ecs_event_t0)),
    };

    ce_array_push(_G.world_array, wi, _G.allocator);

    ce_handler_create(&_G.world_array[idx].entity_handler, _G.allocator);

    ce_hash_add(&_G.world_map, world.h, idx, _G.allocator);
    return &_G.world_array[idx];
}

static ct_world_t0 create_world() {
    ct_world_t0 world = {.h = ce_handler_create(&_G.world_handler,
                                                _G.allocator)};

    CE_ASSERT("ecs", world.h != 0);

    world_instance_t *w = _new_world(world);

    w->world = world;
    w->db = ce_cdb_a0->db();

    return world;
}


ct_ecs_events_t0 events(ct_world_t0 world) {
    world_instance_t *w = get_world_instance(world);

    uint64_t envents_n = atomic_load(&w->events_n);

    return (ct_ecs_events_t0) {.n = envents_n, .events = w->events};
}

static void destroy_world(ct_world_t0 world) {
    ce_handler_destroy(&_G.world_handler, world.h, _G.allocator);
}

static struct ct_ecs_a0 _api = {
        .create_world = create_world,
        .destroy_world = destroy_world,

        .events = events,
        //ENT
        .create = create_entities,
        .destroy = destroy,
        .alive = alive,
        .spawn = spawn_entity,
        .has = has,
        .link = link,
        .parent= parent,
        .first_child = first_child,
        .next_sibling = next_sibling,

        //SIMU
        .simulate = simulate,
        .process = process,

        //COMP
        .get_interface = get_interface,
        .mask = component_mask,
        .get_all = get_all,
        .get_one = get_one,
        .add = add_components,
        .remove = remove_components,
};

struct ct_ecs_a0 *ct_ecs_a0 = &_api;


static void _init_api(struct ce_api_a0 *api) {
    api->register_api(CT_ECS_API, &_api, sizeof(_api));
}

static void _componet_api_add(uint64_t name,
                              void *api) {
    ct_component_i0 *component_i = api;

    ce_array_push(_G.components_name, component_i->cdb_type(), _G.allocator);

    ce_hash_add(&_G.component_interface_map, component_i->cdb_type(),
                (uint64_t) component_i, _G.allocator);

    const uint64_t cid = _G.component_count++;
    ce_hash_add(&_G.component_types, component_i->cdb_type(), cid, _G.allocator);
}



//static uint64_t _get_comp_from_obj(world_instance_t *world,
//                                   ce_cdb_t0 db,
//                                   uint64_t obj) {
//    uint64_t idx = ce_hash_lookup(&world->comp_spawn_map, obj, UINT64_MAX);
//
//    if (idx != UINT64_MAX) {
//        return obj;
//    }
//
//    uint64_t parent = ce_cdb_a0->parent(db, obj);
//    if (!parent) {
//        return 0;
//    }
//
//    return _get_comp_from_obj(world, db, parent);
//}

static void _update(float dt) {
    uint32_t wn = ce_array_size(_G.world_array);
    ct_entity_t0 *ents = NULL;

    //destroy
    uint32_t destroyed_n = 0;
    const uint64_t *destroyed = ce_cdb_a0->destroyed(ce_cdb_a0->db(),
                                                     &destroyed_n);

    for (int j = 0; j < destroyed_n; ++j) {
        uint64_t obj = destroyed[j];

        uint64_t type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), obj);

        if (type == ENTITY_INSTANCE) {
            for (uint32_t i = 0; i < wn; ++i) {
                struct world_instance_t *world = &_G.world_array[i];

                uint64_t idx = ce_hash_lookup(&world->obj_entmap, obj,
                                              UINT64_MAX);
                if (UINT64_MAX == idx) {
                    continue;
                }

                struct spawn_info_t *si = &world->obj_spawn_info[idx];
                destroy(world->world, si->ents, ce_array_size(si->ents));
            }
        } else {

            for (uint32_t i = 0; i < wn; ++i) {
                struct world_instance_t *world = &_G.world_array[i];
                uint64_t idx = ce_hash_lookup(&world->comp_spawn_map, obj,
                                              UINT64_MAX);
                if (UINT64_MAX == idx) {
                    continue;
                }

                struct spawn_info_t *si = &world->comp_spawn_info[idx];

                struct ct_entity_t0 *ents = si->ents;
                uint32_t ents_n = ce_array_size(ents);
                for (int e = 0; e < ents_n; ++e) {
                    struct ct_entity_t0 ent = ents[e];
                    remove_components(world->world, ent, &type, 1);

                }

                _remove_spawn_info(world->comp_spawn_info,
                                   &world->comp_spawn_map, obj);
            }
        }
    }

    // changed
    uint32_t changed_n = 0;
    const uint64_t *orig_changed = ce_cdb_a0->changed_objects(ce_cdb_a0->db(), &changed_n);

    uint64_t *changed = CE_ALLOC(_G.allocator, uint64_t, sizeof(uint64_t) * changed_n);
    memcpy(changed, orig_changed, sizeof(uint64_t) * changed_n);

    for (int j = 0; j < changed_n; ++j) {
        uint64_t obj = changed[j];

        if (!obj) {
            continue;
        }

        const ce_cdb_obj_o0 *obj_r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

        uint64_t parent = ce_cdb_a0->parent(ce_cdb_a0->db(), obj);
        uint64_t type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), obj);

        for (uint32_t i = 0; i < wn; ++i) {
            struct world_instance_t *world = &_G.world_array[i];
            uint64_t idx = ce_hash_lookup(&world->obj_entmap, parent,
                                          UINT64_MAX);
            // is entity?
            if (idx != UINT64_MAX) {
                struct spawn_info_t *si = &world->obj_spawn_info[idx];


                if (type == ENTITY_CHILDREN) {
                    uint32_t change_n = 0;
                    const struct ce_cdb_change_ev_t0 *changes;
                    changes = ce_cdb_a0->changed(obj_r, &change_n);

                    ce_array_clean(ents);
                    uint32_t ents_n = ce_array_size(si->ents);
                    ce_array_push_n(ents, si->ents, ents_n, _G.allocator);
                    
                    for (int ch = 0; ch < change_n; ++ch) {
                        struct ce_cdb_change_ev_t0 ev = changes[ch];

                        if (ev.type == CE_CDB_CHANGE) {
                            uint64_t ent_obj = ev.new_value.subobj;

                            for (int e = 0; e < ents_n; ++e) {
                                ct_entity_t0 new_ents = spawn_entity(world->world, ent_obj);

                                link(world->world, ents[e], new_ents);
                            }
                        } else if (ev.type == CE_CDB_REMOVE) {
                            uint64_t ent_obj = ev.old_value.subobj;

                            uint64_t idx = ce_hash_lookup(&world->obj_entmap,
                                                          ent_obj,
                                                          UINT64_MAX);

                            if (UINT64_MAX == idx) {
                                continue;
                            }

                            ce_array_clean(ents);
                            uint32_t ents_n = ce_array_size(si->ents);
                            ce_array_push_n(ents, si->ents, ents_n, _G.allocator);

                            destroy(world->world, ents, ents_n);
                        }
                    }
                } else if (type == ENTITY_COMPONENTS) {
                    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                             obj);

                    uint32_t change_n = 0;
                    const struct ce_cdb_change_ev_t0 *changes;
                    changes = ce_cdb_a0->changed(r, &change_n);

                    for (int ch = 0; ch < change_n; ++ch) {
                        struct ce_cdb_change_ev_t0 ev = changes[ch];
                        if (ev.type == CE_CDB_CHANGE) {
                            uint64_t comp_obj = ev.new_value.subobj;
                            uint64_t k = ce_cdb_a0->obj_type(ce_cdb_a0->db(), comp_obj);

                            ce_array_clean(ents);
                            uint32_t ents_n = ce_array_size(si->ents);
                            ce_array_push_n(ents, si->ents, ents_n, _G.allocator);

                            for (int e = 0; e < ents_n; ++e) {
                                struct ct_entity_t0 ent = ents[e];

                                uint64_t ent_type = _entity_type(world, ent);
                                uint64_t new_type = combine_component(&k, 1);
                                new_type = ent_type | new_type;

                                if (ent_type == new_type) {
                                    continue;
                                }

                                _add_components_from_obj(world, ent, k, comp_obj);
                            }
                        }
                    }
                }

                continue;
            }
        }
    }

    CE_FREE(_G.allocator, changed);
    ce_array_free(ents, _G.allocator);
}


static uint64_t task_name() {
    return CT_ECS_SYNC_TASK;
}

static uint64_t *update_after(uint64_t *n) {
    static uint64_t a[] = {
            CT_EDITOR_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}

static uint64_t *update_before(uint64_t *n) {
    static uint64_t a[] = {
            CT_GAME_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}

static struct ct_kernel_task_i0 ecs_sync_task = {
        .name = task_name,
        .update = _update,
        .update_after = update_after,
        .update_before = update_before,
};

static uint64_t task_event_name() {
    return CT_ECS_EVENT_TASK;
}

static uint64_t *task_event_update_after(uint64_t *n) {
    static uint64_t a[] = {
            CT_RENDER_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}


static void task_event_update(float dt) {
    uint32_t wn = ce_array_size(_G.world_array);
    for (int i = 0; i < wn; ++i) {
        world_instance_t *w = &_G.world_array[i];
        w->events_n = 0;
    }
}

static struct ct_kernel_task_i0 ecs_event_task = {
        .name = task_event_name,
        .update = task_event_update,
        .update_after = task_event_update_after,
};


static void _init(struct ce_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .db = ce_cdb_a0->db(),
    };

    ce_handler_create(&_G.world_handler, _G.allocator);

    ce_api_a0->register_api(RESOURCE_I, &ct_resource_api, sizeof(ct_resource_api));
    ce_api_a0->register_api(KERNEL_TASK_INTERFACE, &ecs_sync_task, sizeof(ecs_sync_task));
    ce_api_a0->register_api(KERNEL_TASK_INTERFACE, &ecs_event_task, sizeof(ecs_event_task));
    ce_api_a0->register_on_add(COMPONENT_I, _componet_api_add);
}

static ce_cdb_prop_def_t0 entity_prop[] = {
        {.name = "children", .type = CDB_TYPE_SUBOBJECT, .obj_type = ENTITY_CHILDREN},
        {.name = "components", .type = CDB_TYPE_SUBOBJECT, .obj_type = ENTITY_COMPONENTS},
};

void CE_MODULE_LOAD(ecs)(struct ce_api_a0 *api,
                         int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ce_task_a0);
    _init(api);

    ce_cdb_a0->reg_obj_type(ENTITY_INSTANCE, entity_prop, CE_ARRAY_LEN(entity_prop));
}

void CE_MODULE_UNLOAD(ecs)(struct ce_api_a0 *api,
                           int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}