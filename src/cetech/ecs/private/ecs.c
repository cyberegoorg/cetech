//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <stdatomic.h>

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
#include <celib/handler.h>
#include <celib/task.h>
#include <celib/containers/mpmc.h>

#include <cetech/ecs/ecs.h>
#include <cetech/asset/asset.h>
#include <cetech/asset_preview/asset_preview.h>
#include <cetech/kernel/kernel.h>
#include <cetech/renderer/renderer.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/editor.h>
#include <cetech/game/game_system.h>
#include <cetech/parent/parent.h>
#include <cetech/editor/dock.h>
#include <cetech/debugui/debugui.h>
#include <celib/containers/buffer.h>
#include <cetech/asset_io/asset_io.h>

//==============================================================================
// Globals
//==============================================================================

#define MAX_ENTITIES 1000000000
#define CHUNK_SIZE 16384

#define _G ecs_g

#define LOG_WHERE "ecs"

#define _entity_data_idx(w, ent) \
    w->entity_idx[handler_idx((ent).h)]

#define _entity_obj(w, ent) \
    w->entity_obj[handler_idx((ent).h)]

#define _entity_chunk(w, ent) \
    w->entity_chunk[handler_idx((ent).h)]

///
typedef struct spawn_info_t {
    uint64_t ent_obj;
    ct_entity_t0 *ents;
} spawn_info_t;

typedef struct spawn_infos_t {
    ce_hash_t obj_entmap;
    ce_hash_t obj_spawninfo_map;
    spawn_info_t *obj_spawninfo_pool;
    atomic_int obj_spawninfo_pool_n;
    ce_mpmc_queue_t0 obj_spawninfo_free;
} spawn_infos_t;
///

typedef struct ent_chunk_t {
    ct_archemask_t0 archetype_mask;
    uint32_t ent_n;
    uint32_t archetype_idx;
    uint32_t *version;
    struct ent_chunk_t *next;
    struct ent_chunk_t *prev;
} ent_chunk_t;

typedef struct ent_archetype_t {
    ct_archemask_t0 archetype_mask;
    ent_chunk_t *first;
    ce_hash_t comp_idx;
    uint64_t *name;
    uint32_t *offset;
    uint32_t *size;
    uint32_t idx;
    uint32_t max_ent;
    uint32_t component_n;
    bool has_system_state; // TODO FLAG?
} archetype_t;

typedef struct world_instance_t {
    ct_world_t0 world;
    ce_cdb_t0 db;

    const char *name;

    // Entity
    ce_handler_t0 entity_handler;

    uint64_t *entity_idx;
//    uint64_t *entity_obj;
    ent_chunk_t **entity_chunk;

    // Storage
    ce_hash_t component_obj_map;
    spawn_infos_t obj_spawninfo;
    spawn_infos_t comp_spawninfo;

    // Cunk
    ent_chunk_t **chunk_pool;
    ent_chunk_t **chunk_pool_free;

    // Archetype
    uint32_t *archetype_array;
    archetype_t *archetype_pool;
    uint32_t *archetype_free;
    ce_hash_t archetype_map;

    // Version
    uint32_t global_system_version;
    ce_hash_t last_system_version;

    uint32_t global_world_version;
    uint32_t last_world_version;
} world_instance_t;

static struct _G {
    ce_cdb_t0 db;

    // WORLD
    ce_hash_t world_map;
    ce_handler_t0 world_handler;
    world_instance_t *world_array;

    uint32_t component_count;
    ce_hash_t component_types;
    uint64_t *components_name;
    uint64_t system_state_components_mask;
    ce_hash_t component_interface_map;

    // SIM
    ce_hash_t system_map;

    ce_hash_t system_group_map;
    ce_hash_t system_group_g_map;

    uint32_t *graph_pool_free;
    ce_ba_graph_t *graph_pool;

    ct_world_t0 *worlds;
    ce_alloc_t0 *allocator;

    ce_mpmc_queue_t0 *cmd_buf_pool;
    uint32_t *free_cmd_buff_queue;
} _G;

uint32_t _new_graph() {
    uint64_t free_n = ce_array_size(_G.graph_pool_free);

    if (!free_n) {
        uint64_t idx = ce_array_size(_G.graph_pool);
        ce_array_push(_G.graph_pool, (ce_ba_graph_t) {}, _G.allocator);
        return idx;
    }

    uint64_t idx = ce_array_back(_G.graph_pool_free);
    ce_array_pop_back(_G.graph_pool_free);

    return idx;
}

void _free_graph(uint32_t idx) {
    ce_array_push(_G.graph_pool_free, idx, _G.allocator);
}

//
static ct_archemask_t0 combine_component(const uint64_t *component_name,
                                         uint32_t name_count) {
    uint64_t new_type = 0;
    for (int i = 0; i < name_count; ++i) {
        new_type |= (1 << ce_hash_lookup(&_G.component_types, component_name[i], 0));
    }

    return (ct_archemask_t0) {new_type};
}

static struct world_instance_t *get_world_instance(ct_world_t0 world) {
    uint64_t idx = ce_hash_lookup(&_G.world_map, world.h, UINT64_MAX);

    if (UINT64_MAX == idx) {
        return NULL;
    }

    return &_G.world_array[idx];
}

static struct ct_ecs_component_i0 *get_component_interface(uint64_t name) {
    return (ct_ecs_component_i0 *) ce_hash_lookup(&_G.component_interface_map, name, 0);
}

static ct_archemask_t0 component_mask(uint64_t name) {
    return (ct_archemask_t0) {(1llu << ce_hash_lookup(&_G.component_types, name, 0))};
}

static uint64_t component_idx(archetype_t *archetype,
                              uint64_t component_name) {
    return ce_hash_lookup(&archetype->comp_idx, component_name, UINT64_MAX);
}

// CHUNK
ent_chunk_t *_get_new_chunk(world_instance_t *world) {
    uint32_t poolfree_n = ce_array_size(world->chunk_pool_free);

    if (!poolfree_n) {
        ce_log_a0->debug(LOG_WHERE, "Allocate new chunk from memory.");
        ent_chunk_t *chunk = CE_ALLOC(_G.allocator, ent_chunk_t, CHUNK_SIZE);

        memset(chunk, 0, CHUNK_SIZE);

        ce_array_push(world->chunk_pool, chunk, _G.allocator);
        return chunk;
    }

//    ce_log_a0->debug(LOG_WHERE, "Allocate new chunk from pool");

    ent_chunk_t *chunk = ce_array_back(world->chunk_pool_free);
    ce_array_pop_back(world->chunk_pool_free);

    memset(chunk, 0, CHUNK_SIZE);

    return chunk;
}

void _free_chunk(world_instance_t *world,
                 ent_chunk_t *chunk) {
    ce_array_clean(chunk->version);
    ce_array_push(world->chunk_pool_free, chunk, _G.allocator);
}

// ARCHETYPE

static bool _archetype_all(ct_archemask_t0 t1,
                           ct_archemask_t0 t2) {
    return (t1.mask & t2.mask) == t2.mask;
}

static bool _archetype_none(ct_archemask_t0 t1,
                            ct_archemask_t0 t2) {
    return (t1.mask & t2.mask) == 0;
}

static bool _archetype_any(ct_archemask_t0 t1,
                           ct_archemask_t0 t2) {
    return (t1.mask & t2.mask);
}

static ct_archemask_t0 _archetype_add(ct_archemask_t0 t1,
                                      ct_archemask_t0 t2) {
    return (ct_archemask_t0) {.mask=t1.mask | t2.mask};
}

static bool _archetype_eq(ct_archemask_t0 t1,
                          ct_archemask_t0 t2) {
    return t1.mask == t2.mask;
}

archetype_t *_get_new_archetype(world_instance_t *world) {
    uint32_t poolfree_n = ce_array_size(world->archetype_free);

    if (!poolfree_n) {
        ce_log_a0->debug(LOG_WHERE, "Allocate new archetype from memory.");

        uint32_t idx = ce_array_size(world->archetype_pool);
        ce_array_push(world->archetype_pool,
                      (archetype_t) {.idx = idx}, _G.allocator);
        return &world->archetype_pool[idx];
    }

    // ce_log_a0->debug(LOG_WHERE, "Allocate new archetype from pool");

    uint32_t idx = ce_array_back(world->archetype_free);
    ce_array_pop_back(world->archetype_free);
    return &world->archetype_pool[idx];
}

void _free_archetype(world_instance_t *world,
                     archetype_t *archetype) {

    ce_array_clean(archetype->size);
    ce_array_clean(archetype->name);
    ce_array_clean(archetype->offset);
    ce_hash_clean(&archetype->comp_idx);

    archetype->archetype_mask.mask = 0;

    const uint32_t n = ce_array_size(world->archetype_array);
    for (int i = 0; i < n; ++i) {
        if (world->archetype_array[i] != archetype->idx) {
            continue;
        }

        const uint32_t last_idx = n - 1;
        world->archetype_array[i] = world->archetype_array[last_idx];
        ce_array_pop_back(world->archetype_array);
        break;
    }

    ent_chunk_t *chunk = archetype->first;

    while (chunk) {
        _free_chunk(world, chunk);
        chunk = chunk->next;
    }

    ce_array_push(world->archetype_free, archetype->idx, _G.allocator);
}

ct_entity_t0 *_get_entity_array(ent_chunk_t *chunk) {
    return (ct_entity_t0 *) (chunk + 1);
}

void *_get_component_array(archetype_t *storage,
                           ent_chunk_t *chunk,
                           uint64_t comp_idx) {
    if (comp_idx == UINT64_MAX) {
        return NULL;
    }

    void *data = (chunk + 1);

    uint32_t offset = storage->offset[comp_idx];

    if (!offset) {
        return NULL;
    }

    return data + storage->offset[comp_idx];
}

archetype_t *_get_archetype(world_instance_t *w,
                            ct_archemask_t0 archetype) {
    uint64_t archetype_idx = ce_hash_lookup(&w->archetype_map, archetype.mask, UINT64_MAX);

    if (UINT64_MAX == archetype_idx) {
        return NULL;
    }

    archetype_t *storage = &w->archetype_pool[archetype_idx];
    return storage;
}

static void *get_one(ct_world_t0 world,
                     uint64_t component_name,
                     ct_entity_t0 entity,
                     bool write) {

    if (!entity.h) {
        return NULL;
    }

    world_instance_t *w = get_world_instance(world);

    ent_chunk_t *chunk = _entity_chunk(w, entity);
    if (!chunk) {
        return NULL;
    }

    archetype_t *storage = _get_archetype(w, chunk->archetype_mask);

    if (!storage) {
        return NULL;
    }


    uint64_t com_idx = component_idx(storage, component_name);

    if (com_idx == UINT64_MAX) {
        return NULL;
    }

    if (write) {
        chunk->version[com_idx] = w->global_system_version;
    }

    uint32_t data_idx = _entity_data_idx(w, entity);


    void *data = _get_component_array(storage, chunk, com_idx);
    if (!data) {
        return NULL;
    }

    return data + (storage->size[com_idx] * data_idx);
}

void _add_to_archetype(world_instance_t *w,
                       ct_entity_t0 ent,
                       ct_archemask_t0 archetype_mask) {
    ent_chunk_t *ch = _entity_chunk(w, ent);

    if (ch && (ch->archetype_mask.mask == archetype_mask.mask)) {
        return;
    }

    uint64_t archetype_idx = ce_hash_lookup(&w->archetype_map, archetype_mask.mask, UINT64_MAX);

    if (UINT64_MAX == archetype_idx) {
        archetype_t *storage = _get_new_archetype(w);
        archetype_idx = storage->idx;
        ce_hash_add(&w->archetype_map, archetype_mask.mask, archetype_idx, _G.allocator);

        ce_array_push(w->archetype_array, archetype_idx, _G.allocator);

        bool has_system_state = false;
        uint32_t all_component_size = sizeof(ct_entity_t0);
        const uint32_t component_n = ce_array_size(_G.components_name);

        uint32_t comp_idx = 0;
        for (int i = 0; i < component_n; ++i) {
            uint64_t component_name = _G.components_name[i];

            ct_ecs_component_i0 *ci = get_component_interface(component_name);

            ct_archemask_t0 mask = component_mask(component_name);
            if (!(_archetype_any(archetype_mask, mask))) {
                continue;
            }

            ce_hash_add(&storage->comp_idx, component_name, comp_idx, _G.allocator);
            comp_idx += 1;

            if (ci->is_system_state) {
                has_system_state = true;
            }

            if (ci->size) {
                all_component_size += ci->size;
            }

            ce_array_push(storage->size, ci->size, _G.allocator);
            ce_array_push(storage->name, component_name, _G.allocator);
            ce_array_push(storage->offset, 0, _G.allocator);
        }

        uint32_t free_space = CHUNK_SIZE - sizeof(ent_chunk_t);
        storage->archetype_mask = archetype_mask;
        storage->max_ent = free_space / all_component_size;
        storage->has_system_state = has_system_state;
        storage->component_n = comp_idx;

        // offsets
        uint32_t data_offset = (sizeof(ct_entity_t0) * storage->max_ent);

        for (int i = 0; i < comp_idx; ++i) {
            if (storage->size[i]) {
                storage->offset[i] = data_offset;
                data_offset += storage->size[i] * storage->max_ent;
            }
        }

        ent_chunk_t *chunk = _get_new_chunk(w);
        chunk->archetype_idx = storage->idx;
        chunk->archetype_mask = storage->archetype_mask;
        ce_array_resize(chunk->version, storage->component_n, _G.allocator);
        for (int j = 0; j < storage->component_n; ++j) {
            chunk->version[j] = w->global_system_version;
        }
        storage->first = chunk;
    }

    archetype_t *storage = &w->archetype_pool[archetype_idx];

    // Find free chunk
    ent_chunk_t *chunk = storage->first;
    while (chunk) {
        if ((chunk->ent_n + 1) >= storage->max_ent) {
            if (chunk->next) {
                chunk = chunk->next;
                continue;
            }

            ent_chunk_t *new = _get_new_chunk(w);
            new->archetype_idx = storage->idx;
            new->archetype_mask = storage->archetype_mask;
            ce_array_resize(new->version, storage->component_n, _G.allocator);
            for (int j = 0; j < storage->component_n; ++j) {
                new->version[j] = w->global_system_version;
            }

            new->next = storage->first;
            storage->first->prev = new;
            storage->first = new;
            chunk = new;
            break;
        } else {
            break;
        }
    }

    const uint64_t ent_data_idx = chunk->ent_n++;

    ct_entity_t0 *entity = _get_entity_array(chunk);

    entity[ent_data_idx] = ent;

    for (int i = 0; i < storage->component_n; ++i) {
        chunk->version[i] = w->global_system_version;

        uint32_t size = storage->size[i];
        if (!size) {
            continue;
        }

        void *data = _get_component_array(storage, chunk, i);
        memset(data + (ent_data_idx * size), 0, size);
    }

    _entity_data_idx(w, ent) = ent_data_idx;
    _entity_chunk(w, ent) = chunk;
}

void _remove_from_archetype(world_instance_t *w,
                            ct_entity_t0 ent,
                            uint64_t ent_idx,
                            ent_chunk_t *chunk,
                            ct_archemask_t0 archetype) {
    if (!chunk->ent_n) {
        return;
    }

    uint32_t last_idx = --chunk->ent_n;

    uint64_t entity_data_idx = ent_idx;

    archetype_t *storage = &w->archetype_pool[chunk->archetype_idx];

    if (!chunk->ent_n) {
        if (chunk->prev) {
            chunk->prev->next = chunk->next;
        }

        if (chunk->next) {
            chunk->next->prev = chunk->prev;
        }

        if (storage->first != chunk) {
            _free_chunk(w, chunk);
        } else if ((storage->first == chunk) && (!chunk->next)) {
            ce_hash_remove(&w->archetype_map, archetype.mask);
            _free_archetype(w, storage);
        } else if (storage->first == chunk) {
            storage->first = chunk->next;
            _free_chunk(w, chunk);
        }
        return;
    }

    if (last_idx == entity_data_idx) {
        return;
    }

    ct_entity_t0 *entity = _get_entity_array(chunk);

    ct_entity_t0 last_ent = entity[last_idx];
    _entity_data_idx(w, last_ent) = entity_data_idx;

    entity[entity_data_idx] = last_ent;

    for (int i = 0; i < storage->component_n; ++i) {
        chunk->version[i] = w->global_system_version;

        void *data = _get_component_array(storage, chunk, i);

        if (!data) {
            continue;
        }

        uint64_t size = storage->size[i];

        memcpy(data + (entity_data_idx * size),
               data + (last_idx * size), size);
    }
}

static void _move_data_from_archetype(world_instance_t *w,
                                      struct ct_entity_t0 ent,
                                      ent_chunk_t *old_chunk,
                                      uint32_t old_idx,
                                      ct_archemask_t0 ent_type,
                                      ct_archemask_t0 new_type) {

    archetype_t *old_storage = _get_archetype(w, ent_type);
    archetype_t *new_storage = _get_archetype(w, new_type);

    CE_ASSERT(LOG_WHERE, old_storage != new_storage);
    CE_ASSERT(LOG_WHERE, old_storage);
    CE_ASSERT(LOG_WHERE, new_storage);

    uint32_t idx = _entity_data_idx(w, ent);
    ent_chunk_t *chunk = _entity_chunk(w, ent);

    for (int i = 0; i < old_storage->component_n; ++i) {
        uint64_t component_name = old_storage->name[i];

        ct_archemask_t0 mask = component_mask(component_name);
        if (_archetype_none(ent_type, mask)) {
            continue;
        }

        if (_archetype_none(new_type, mask)) {
            continue;
        }

        uint32_t size = old_storage->size[i];
        if (!size) {
            continue;
        }

        void *old_data = _get_component_array(old_storage, old_chunk, i);

        uint64_t comp_idx = component_idx(new_storage, component_name);
        void *new_data = _get_component_array(new_storage, chunk, comp_idx);

        CE_ASSERT(LOG_WHERE, old_data != new_data);

        memcpy(new_data + (idx * size),
               old_data + (old_idx * size),
               size);
    }
}

void _add_components_to_archetype(ct_world_t0 world,
                                  struct ct_entity_t0 ent,
                                  ct_archemask_t0 new_type) {
    world_instance_t *w = get_world_instance(world);

    uint32_t idx = _entity_data_idx(w, ent);
    ent_chunk_t *chunk = _entity_chunk(w, ent);

    if (chunk) {
        if (_archetype_eq(chunk->archetype_mask, new_type)) {
            return;
        }
        new_type = _archetype_add(new_type, chunk->archetype_mask);
    }


    _add_to_archetype(w, ent, new_type);

    if (chunk && chunk->archetype_mask.mask) {
        _move_data_from_archetype(w, ent, chunk, idx, chunk->archetype_mask, new_type);
        _remove_from_archetype(w, ent, idx, chunk, chunk->archetype_mask);
    }
}

///

#define ADD_COMPONENT_CMD \
    CE_ID64_0("add_component", 0xd0363f18e7a5b7e2ULL)

#define REMOVE_COMPONENT_CMD \
    CE_ID64_0("remove_component", 0x1845aca1baf10397ULL)

typedef struct add_component_t {
    uint64_t type;
    ct_world_t0 world;
    ct_entity_t0 ent;
    uint64_t component_type;
    void *data;
    uint32_t component_size;
} add_component_t;

typedef struct remove_components_t {
    ct_world_t0 world;
    ct_entity_t0 ent;
    uint64_t *components;
    uint32_t n;
} remove_components_t;

typedef struct cmd_t {
    uint64_t type;
    union {
        add_component_t add;
        remove_components_t remove;
    };
} cmd_t;

uint32_t _new_cmd_buff() {
    if (ce_array_empty(_G.free_cmd_buff_queue)) {
        uint32_t idx = ce_array_size(_G.cmd_buf_pool);
        ce_array_push(_G.cmd_buf_pool, (ce_mpmc_queue_t0) {}, _G.allocator);

        ce_mpmc_queue_t0 *q = &_G.cmd_buf_pool[idx];
        ce_mpmc_init(q, 4096, sizeof(cmd_t), _G.allocator);

        return idx;
    }

    uint32_t idx = ce_array_back(_G.free_cmd_buff_queue);
    ce_array_pop_back(_G.free_cmd_buff_queue);
    return idx;
}

void _free_cmd_buff(uint32_t idx) {
    ce_mpmc_queue_t0 *q = &_G.cmd_buf_pool[idx];
    ce_mpmc_clean(q);

    ce_array_push(_G.free_cmd_buff_queue, idx, _G.allocator);
}

static void *virtual_alloc(uint64_t size) {
    return CE_REALLOC(ce_memory_a0->virt_system, void, NULL, size, 0);
}

//static void virtual_free(void* ptr, uint64_t size) {
//    munmap(ptr, size);
//}

static void _init_spawn_infos(spawn_infos_t *infos) {
    infos->obj_spawninfo_pool = virtual_alloc(sizeof(spawn_info_t) * MAX_ENTITIES);
    ce_mpmc_init(&infos->obj_spawninfo_free, 4096, sizeof(uint64_t), _G.allocator);
}

static void _add_spawn_obj(ce_cdb_t0 db,
                           spawn_infos_t *infos,
                           uint64_t obj,
                           struct ct_entity_t0 ent) {
    CE_ASSERT("ecs", ent.h != 0);

    uint64_t idx = ce_hash_lookup(&infos->obj_entmap, obj, UINT64_MAX);

    if (idx == UINT64_MAX) {
        ce_hash_add(&infos->obj_entmap, obj, ent.h, _G.allocator);
    }

    uint64_t spawninfo_idx = ce_hash_lookup(&infos->obj_spawninfo_map, obj, UINT64_MAX);
    if (spawninfo_idx == UINT64_MAX) {
        if (!ce_mpmc_dequeue(&infos->obj_spawninfo_free, &spawninfo_idx)) {
            spawninfo_idx = atomic_fetch_add(&infos->obj_spawninfo_pool_n, 1);
            infos->obj_spawninfo_pool[spawninfo_idx] = (spawn_info_t) {};
        }

        ce_hash_add(&infos->obj_spawninfo_map, obj, spawninfo_idx, _G.allocator);
    }

    spawn_info_t *spawn_info = &infos->obj_spawninfo_pool[spawninfo_idx];
    spawn_info->ent_obj = obj;

    ce_array_push(spawn_info->ents, ent, _G.allocator);
}

static void _add_ent_spawn_obj(ce_cdb_t0 db,
                               world_instance_t *world,
                               uint64_t obj,
                               struct ct_entity_t0 ent) {
    _add_spawn_obj(db, &world->obj_spawninfo, obj, ent);
}

static void _add_comp_spawn_obj(ce_cdb_t0 db,
                                world_instance_t *world,
                                uint64_t obj,
                                struct ct_entity_t0 ent) {
    _add_spawn_obj(db, &world->comp_spawninfo, obj, ent);
}

static spawn_info_t *_get_spawninfo(spawn_infos_t *infos,
                                    uint64_t obj) {
    uint64_t spawninfo_idx = ce_hash_lookup(&infos->obj_spawninfo_map, obj, UINT64_MAX);
    if (UINT64_MAX == spawninfo_idx) {
        return NULL;
    }

    spawn_info_t *spawn_info = &infos->obj_spawninfo_pool[spawninfo_idx];
    return spawn_info;
}

void _free_spawninfo(spawn_infos_t *infos,
                     uint64_t obj) {
    uint64_t spawninfo_idx = ce_hash_lookup(&infos->obj_spawninfo_map, obj, UINT64_MAX);
    if (UINT64_MAX == spawninfo_idx) {
        return;
    }

    spawn_info_t *spawn_info = &infos->obj_spawninfo_pool[spawninfo_idx];
    ce_array_clean(spawn_info->ents);
    ce_mpmc_enqueue(&infos->obj_spawninfo_free, &spawninfo_idx);
}

void _free_spawninfo_ent(spawn_infos_t *infos,
                         uint64_t obj,
                         ct_entity_t0 ent) {
    uint64_t spawninfo_idx = ce_hash_lookup(&infos->obj_spawninfo_map, obj, UINT64_MAX);
    if (UINT64_MAX == spawninfo_idx) {
        return;
    }

    spawn_info_t *spawn_info = &infos->obj_spawninfo_pool[spawninfo_idx];

    uint32_t ent_n = ce_array_size(spawn_info->ents);
    for (int i = 0; i < ent_n; ++i) {
        if (spawn_info->ents[i].h == ent.h) {
            spawn_info->ents[i] = spawn_info->ents[ent_n - 1];
            ce_array_pop_back(spawn_info->ents);
            return;
        }
    }
}

static void *get_all(ct_world_t0 world,
                     uint64_t component_name,
                     ct_ecs_ent_chunk_o0 *_item) {
    ent_chunk_t *chunk = (ent_chunk_t *) _item;

    world_instance_t *w = get_world_instance(world);
    archetype_t *storage = &w->archetype_pool[chunk->archetype_idx];

    uint64_t comp_idx = component_idx(storage, component_name);

    return _get_component_array(storage, chunk, comp_idx);
}

static bool has(ct_world_t0 world,
                struct ct_entity_t0 ent,
                uint64_t *component_name,
                uint32_t name_count) {
    world_instance_t *w = get_world_instance(world);

    ent_chunk_t *chunk = _entity_chunk(w, ent);

    if (!chunk) {
        return false;
    }

    ct_archemask_t0 ent_type = chunk->archetype_mask;

    uint64_t mask = 0;
    for (int i = 0; i < name_count; ++i) {
        mask |= (1 << ce_hash_lookup(&_G.component_types, component_name[i], 0));
    }

    return ((ent_type.mask & mask) == mask);
}


static ct_archemask_t0 combine_component_obj(ce_cdb_t0 db,
                                             const uint64_t *component_obj,
                                             uint32_t name_count) {
    uint64_t new_type = 0;
    for (int i = 0; i < name_count; ++i) {
        uint64_t type = ce_cdb_a0->obj_type(db, component_obj[i]);
        new_type |= (1 << ce_hash_lookup(&_G.component_types, type, 0));
    }

    return (ct_archemask_t0) {new_type};
}

static void add_components(ct_world_t0 world,
                           struct ct_entity_t0 ent,
                           const ct_component_pair_t0 *components,
                           uint32_t components_count) {
    uint64_t types[components_count];
    for (int i = 0; i < components_count; ++i) {
        types[i] = components[i].type;
    }
    ct_archemask_t0 component_mask = combine_component(types, components_count);

    world_instance_t *w = get_world_instance(world);
    ent_chunk_t *chunk = _entity_chunk(w, ent);
    if (chunk) {
        component_mask = _archetype_add(chunk->archetype_mask, component_mask);
    }

    _add_components_to_archetype(world, ent, component_mask);

    for (int i = 0; i < components_count; ++i) {
        if (components[i].data) {
            uint8_t *comp_data = get_one(world, types[i], ent, false);

            if (!comp_data) {
                continue;
            }

            ct_ecs_component_i0 *ci = get_component_interface(types[i]);
            uint64_t component_size = ci->size;
            memcpy(comp_data, components[i].data, component_size);
        }
    }
}

static void remove_buff(ct_ecs_cmd_buffer_t *buffer,
                        ct_world_t0 world,
                        ct_entity_t0 ent,
                        const uint64_t *component_name,
                        uint32_t name_count) {
    ce_mpmc_queue_t0 *b = (ce_mpmc_queue_t0 *) buffer;

    for (int i = 0; i < name_count; ++i) {
        void *new_data = NULL;

        new_data = CE_ALLOC(_G.allocator, char, sizeof(uint64_t) * name_count);
        memcpy(new_data, component_name, sizeof(uint64_t) * name_count);

        ce_mpmc_enqueue(b, &(cmd_t) {
                .type = REMOVE_COMPONENT_CMD,
                .remove = {
                        .world = world,
                        .components = new_data,
                        .n = name_count,
                        .ent = ent
                },
        });
    }
}

static void add_buff(ct_ecs_cmd_buffer_t *buffer,
                     ct_world_t0 world,
                     ct_entity_t0 ent,
                     const ct_component_pair_t0 *components,
                     uint32_t components_count) {
    ce_mpmc_queue_t0 *b = (ce_mpmc_queue_t0 *) buffer;

    for (int i = 0; i < components_count; ++i) {
        ct_ecs_component_i0 *ci = get_component_interface(components[i].type);

        void *new_data = NULL;

        if (ci->size) {
            new_data = CE_ALLOC(_G.allocator, char, ci->size);
            memcpy(new_data, components[i].data, ci->size);
        }

        ce_mpmc_enqueue(b, &(cmd_t) {
                .type = ADD_COMPONENT_CMD,
                .add = {
                        .world = world,
                        .data = new_data,
                        .component_type = components[i].type,
                        .component_size = ci->size,
                        .ent = ent
                },
        });
    }
}


static void _add_components_from_obj(world_instance_t *world,
                                     ce_cdb_t0 db,
                                     struct ct_entity_t0 ent,
                                     uint64_t obj) {
    uint64_t mew_component = obj;
    ce_hash_add(&world->component_obj_map, mew_component, mew_component, _G.allocator);

    uint64_t component_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), obj);

    ct_ecs_component_i0 *ci = get_component_interface(component_type);

    if (!ci) {
        return;
    }
    ct_archemask_t0 new_type = combine_component(&component_type, 1);
    _add_components_to_archetype(world->world, ent, new_type);

    uint8_t *comp_data = get_one(world->world, component_type, ent, false);

    if (ci->from_cdb_obj) {
        ci->from_cdb_obj(world->world, db, obj, comp_data);
    }

    _add_comp_spawn_obj(db, world, obj, ent);
}

static void remove_components(ct_world_t0 world,
                              struct ct_entity_t0 ent,
                              const uint64_t *component_name,
                              uint32_t name_count) {
    world_instance_t *w = get_world_instance(world);

    ent_chunk_t *chunk = _entity_chunk(w, ent);

    ct_archemask_t0 new_type = chunk->archetype_mask;
    ct_archemask_t0 comp_type = combine_component(component_name, name_count);

    if (_archetype_none(new_type, comp_type)) {
        return;
    }

    new_type.mask &= ~(comp_type.mask);

    archetype_t *storage = _get_archetype(w, chunk->archetype_mask);

    if (!storage) {
        return;
    }

    uint32_t idx = _entity_data_idx(w, ent);

    if (new_type.mask) {
        _add_to_archetype(w, ent, new_type);
        _move_data_from_archetype(w, ent, chunk, idx, chunk->archetype_mask, new_type);
    }

    _remove_from_archetype(w, ent, idx, chunk, chunk->archetype_mask);

    if (!new_type.mask) {
        if (storage->has_system_state) {
            ce_handler_destroy(&w->entity_handler, ent.h, _G.allocator);
        }
    }
}


static void _execute_cmd(ce_mpmc_queue_t0 *buffer) {
    cmd_t cmd;
    while (ce_mpmc_dequeue(buffer, &cmd)) {
        if (cmd.type == ADD_COMPONENT_CMD) {
            add_component_t *add_cmd = &cmd.add;

            add_components(add_cmd->world, add_cmd->ent,
                           (ct_component_pair_t0[]) {
                                   {
                                           .type = add_cmd->component_type,
                                           .data = add_cmd->data
                                   }
                           }, 1);

            if (add_cmd->data) {
                CE_FREE(_G.allocator, add_cmd->data);
            }
        } else if (cmd.type == REMOVE_COMPONENT_CMD) {
            remove_components_t *remove_cmd = &cmd.remove;
            remove_components(remove_cmd->world, remove_cmd->ent,
                              remove_cmd->components, remove_cmd->n);

            CE_FREE(_G.allocator, remove_cmd->components);
        }
    }
}

typedef struct process_data_t {
    ct_world_t0 world;
    ct_entity_t0 *ents;
    ct_ecs_ent_chunk_o0 *chunk;
    uint64_t count;
    void *data;
    ct_ecs_foreach_fce_t fce;
} process_data_t;

static void _process_task(void *data) {
    process_data_t *pdata = data;
    pdata->fce(pdata->world, pdata->ents, pdata->chunk, pdata->count, pdata->data);
}

static bool _can_run_query_on_archetype(ct_archemask_t0 mask,
                                        const ct_ecs_query_t0 *query) {
    if (query->all.mask) {
        if (!_archetype_all(mask, query->all)) {
            return false;
        }
    }

    if (query->none.mask) {
        if (!_archetype_none(mask, query->none)) {
            return false;
        }
    }

    if (query->any.mask) {
        if (_archetype_any(mask, query->any) == 0) {
            return false;
        }
    }

    return true;
}

static bool chunk_changed(uint32_t version,
                          uint32_t rq_version) {
    if (!rq_version) {
        return true;
    }

    return (int32_t) (version - rq_version) > 0;
}

static bool _need_process_chunk(archetype_t *storage,
                                ent_chunk_t *chunk,
                                const ct_ecs_query_t0 *query,
                                uint64_t rq_version,
                                world_instance_t *w) {
    uint32_t ent_n = chunk->ent_n;

    if (!ent_n) {
        return false;
    }

    ct_archemask_t0 used_components = _archetype_add(query->all, query->any);

    if (query->only_changed) {
        bool chunk_comp_changed = false;
        for (int j = 0; j < storage->component_n; ++j) {
            uint64_t comp_name = storage->name[j];
            ct_archemask_t0 comp_mask = component_mask(comp_name);

            if (!_archetype_any(used_components, comp_mask)) {
                continue;
            }

            if (_archetype_none(query->write, comp_mask)) {
                chunk_comp_changed |= chunk_changed(chunk->version[j], rq_version);
            }
        }

        if (!chunk_comp_changed) {
            return false;
        }
    }

    // Update version for write component.
    for (int j = 0; j < storage->component_n; ++j) {
        uint64_t comp_name = storage->name[j];
        ct_archemask_t0 comp_mask = component_mask(comp_name);

        if (_archetype_any(query->write, comp_mask)) {
            chunk->version[j] = rq_version;
        }
    }

    return true;
}

static void process_query(ct_world_t0 world,
                          ct_ecs_query_t0 query,
                          uint32_t rq_version,
                          ct_ecs_foreach_fce_t fce,
                          void *data) {
    world_instance_t *w = get_world_instance(world);

    const uint32_t type_count = ce_array_size(w->archetype_array);

    ce_task_item_t0 *tasks = NULL;
    process_data_t *task_data = NULL;

    for (int i = 0; i < type_count; ++i) {
        archetype_t *storage = &w->archetype_pool[w->archetype_array[i]];

        ent_chunk_t *chunk = storage->first;

        if (!_can_run_query_on_archetype(storage->archetype_mask, &query)) {
            continue;
        }

        while (chunk) {
            if (!_need_process_chunk(storage, chunk, &query, rq_version, w)) {
                chunk = chunk->next;
                continue;
            }

            ct_entity_t0 *ents = _get_entity_array(chunk);

            uint32_t idx = ce_array_size(task_data);
            ce_array_push(task_data, ((process_data_t) {
                    .world = world,
                    .ents = ents,
                    .chunk = (ct_ecs_ent_chunk_o0 *) chunk,
                    .count = chunk->ent_n,
                    .data = data,
                    .fce = fce,
            }), _G.allocator);

            ce_array_push(tasks, ((ce_task_item_t0) {
                    .data = &task_data[idx],
                    .name = "ecs_process",
                    .work = _process_task
            }), _G.allocator);

            chunk = chunk->next;
        }
    }

    ce_task_counter_t0 *counter = NULL;
    ce_task_a0->add(tasks, ce_array_size(tasks), &counter);
    ce_task_a0->wait_for_counter(counter, 0);

    ce_array_free(task_data, _G.allocator);
    ce_array_free(tasks, _G.allocator);
}


static void process_query_serial(ct_world_t0 world,
                                 ct_ecs_query_t0 query,
                                 uint32_t rq_version,
                                 ct_ecs_foreach_fce_t fce,
                                 void *data) {
    world_instance_t *w = get_world_instance(world);

    const uint32_t type_count = ce_array_size(w->archetype_array);

    for (int i = 0; i < type_count; ++i) {
        archetype_t *storage = &w->archetype_pool[w->archetype_array[i]];

        ent_chunk_t *chunk = storage->first;

        if (!_can_run_query_on_archetype(storage->archetype_mask, &query)) {
            continue;
        }

        while (chunk) {
            if (!_need_process_chunk(storage, chunk, &query, rq_version, w)) {
                chunk = chunk->next;
                continue;
            }

            ct_entity_t0 *ents = _get_entity_array(chunk);

            fce(world, ents, (ct_ecs_ent_chunk_o0 *) chunk, chunk->ent_n, data);

            chunk = chunk->next;
        }
    }
}

uint64_t _get_component_obj(world_instance_t *world,
                            uint64_t obj) {
    if (ce_hash_lookup(&world->component_obj_map, obj, 0)) {
        return obj;
    }

    uint64_t parent = ce_cdb_a0->parent(ce_cdb_a0->db(), obj);
    if (!parent) {
        return 0;
    }

    return _get_component_obj(world, parent);
}

spawn_info_t *_get_component_spawninfo(world_instance_t *world,
                                       uint64_t obj) {
    spawn_info_t *si = _get_spawninfo(&world->comp_spawninfo, obj);

    if (si) {
        return si;
    }

    uint64_t parent = ce_cdb_a0->parent(ce_cdb_a0->db(), obj);
    if (!parent) {
        return 0;
    }

    return _get_component_spawninfo(world, parent);
}

static void _build_graphs() {
    ce_hash_clean(&_G.system_map);
    ce_hash_clean(&_G.system_group_map);
    ce_hash_clean(&_G.system_group_g_map);

    uint64_t *graphs = NULL;

    ce_api_entry_t0 it = ce_api_a0->first(CT_ECS_SYSTEM_GROUP_I0);
    while (it.api) {
        ct_system_group_i0 *i = (it.api);

        uint64_t name = i->name;

        ce_hash_add(&_G.system_group_map, name, (uint64_t) i, _G.allocator);

        uint64_t graph_idx = ce_hash_lookup(&_G.system_group_g_map, name, UINT64_MAX);
        if (UINT64_MAX == graph_idx) {
            graph_idx = _new_graph();
            ce_hash_add(&_G.system_group_g_map, name, graph_idx, _G.allocator);
            ce_array_push(graphs, graph_idx, _G.allocator);
        }

        if (i->group) {
            uint64_t group_graph_idx = ce_hash_lookup(&_G.system_group_g_map, i->group, UINT64_MAX);
            if (group_graph_idx == UINT64_MAX) {
                group_graph_idx = _new_graph();
                ce_hash_add(&_G.system_group_g_map, i->group, group_graph_idx, _G.allocator);
                ce_array_push(graphs, group_graph_idx, _G.allocator);
            }

            ce_ba_graph_t *g = &_G.graph_pool[group_graph_idx];

            ce_bag_add(g, name,
                       i->before.ptr, i->before.len,
                       i->after.ptr, i->after.len,
                       _G.allocator);
        }

        it = ce_api_a0->next(it);
    }

    it = ce_api_a0->first(CT_ECS_SYSTEM_I0);
    while (it.api) {
        struct ct_system_i0 *i = (it.api);

        uint64_t name = i->name;

        ce_hash_add(&_G.system_map, name, (uint64_t) i, _G.allocator);

        if (!i->group) {
            i->group = CT_ECS_SIMULATION_GROUP;
        }

        uint64_t graph_idx = ce_hash_lookup(&_G.system_group_g_map, i->group, UINT64_MAX);
        if (graph_idx != UINT64_MAX) {
            ce_ba_graph_t *g = &_G.graph_pool[graph_idx];
            ce_bag_add(g, name,
                       i->before.ptr, i->before.len,
                       i->after.ptr, i->after.len,
                       _G.allocator);
        }


        it = ce_api_a0->next(it);
    }

    uint64_t graph_n = ce_array_size(graphs);
    for (uint64_t j = 0; j < graph_n; ++j) {
        ce_ba_graph_t *g = &_G.graph_pool[graphs[j]];
        ce_bag_build(g, _G.allocator);
    }

    ce_array_free(graphs, _G.allocator);
}

static void _process_group(ct_world_t0 world,
                           uint64_t group_name,
                           float dt) {
    world_instance_t *w = get_world_instance(world);

    uint64_t group_graph = ce_hash_lookup(&_G.system_group_g_map, group_name, UINT64_MAX);
    if (group_graph == UINT64_MAX) {
        return;
    }

    ce_ba_graph_t *g = &_G.graph_pool[group_graph];
    uint64_t *outputs = g->output;

    const uint64_t systems_n = ce_array_size(outputs);
    for (int i = 0; i < systems_n; ++i) {
        ct_system_i0 *sys = (ct_system_i0 *) ce_hash_lookup(&_G.system_map, outputs[i], 0);

        ct_system_group_i0 *sysg = (ct_system_group_i0 *) ce_hash_lookup(&_G.system_group_map,
                                                                         outputs[i], 0);

        if (sys) {
            uint32_t cmd_buf_idx = _new_cmd_buff();
            ce_mpmc_queue_t0 *buff = &_G.cmd_buf_pool[cmd_buf_idx];

            w->global_system_version++;
            if (w->global_system_version == 0) {
                w->global_system_version++;
            }

            uint32_t rq_version = ce_hash_lookup(&w->last_system_version, outputs[i], 0);

            if (sys->process) {
                sys->process(w->world, dt, rq_version, (ct_ecs_cmd_buffer_t *) buff);
            }

            ce_hash_add(&w->last_system_version, outputs[i], w->global_system_version,
                        _G.allocator);

            _execute_cmd(buff);
            _free_cmd_buff(cmd_buf_idx);
        } else if (sysg) {
            _process_group(world, sysg->name, dt);
        }

    }

}

static void step(ct_world_t0 world,
                 float dt) {

    world_instance_t *w = get_world_instance(world);
    if (w->global_world_version != w->last_world_version) {
        w->last_world_version = w->global_world_version;
        _build_graphs();
    }

    _process_group(world, CT_ECS_SIMULATION_GROUP, dt);
    _process_group(world, CT_ECS_PRESENTATION_GROUP, dt);
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

//        w->entity_obj[idx] = 0;
        w->entity_chunk[idx] = NULL;
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

//        uint64_t obj = objs[i];
//        w->entity_obj[idx] = obj;
        w->entity_chunk[idx] = NULL;
    }
}


static void destroy(ct_world_t0 world,
                    struct ct_entity_t0 *entity,
                    uint32_t count) {
    world_instance_t *w = get_world_instance(world);

    for (uint32_t i = 0; i < count; ++i) {
        ct_entity_t0 ent = entity[i];

        uint64_t data_idx = _entity_data_idx(w, ent);
        ent_chunk_t *chunk = _entity_chunk(w, ent);

        ct_child_c *child = get_one(world, CT_CHILD_COMPONENT, ent, false);

        archetype_t *storage = NULL;
        if (chunk) {
            storage = _get_archetype(w, chunk->archetype_mask);
            if ((storage) && storage->has_system_state) {
                ct_archemask_t0 new_type = {
                        chunk->archetype_mask.mask & _G.system_state_components_mask};

                if (new_type.mask == chunk->archetype_mask.mask) {
                    continue;
                }

                _add_to_archetype(w, ent, new_type);
                _move_data_from_archetype(w, ent, chunk, data_idx, chunk->archetype_mask, new_type);
            }
            _remove_from_archetype(w, ent, data_idx, chunk, chunk->archetype_mask);
        } else {
            CE_UNUSED(chunk);
        }

        if (child) {
            destroy(world, child->child, ce_array_size(child->child));
        }

        if (storage && !storage->has_system_state) {
            ce_handler_destroy(&w->entity_handler, ent.h, _G.allocator);
        }

//        uint64_t ent_obj = _entity_obj(w, ent);
//
//        const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), ent_obj);
//        uint64_t components_n = ce_cdb_a0->read_objset_num(r, ENTITY_COMPONENTS);
//        uint64_t components_keys[components_n];
//        ce_cdb_a0->read_objset(r, ENTITY_COMPONENTS, components_keys);
//        for (int j = 0; j < components_n; ++j) {
//            _free_spawninfo_ent(&w->comp_spawninfo, components_keys[j], ent);
//        }
//
//        _free_spawninfo_ent(&w->obj_spawninfo, ent_obj, ent);
    }
}

static uint64_t cdb_type() {
    return ENTITY_TYPE;
}

static struct ct_entity_t0 spawn_entity(ct_world_t0 world,
                                        uint64_t name);

static struct ct_entity_t0 load(uint64_t asset,
                                ct_world_t0 world) {

    ct_entity_t0 ent = spawn_entity(world, asset);

    return ent;
}


void *asset_get_interface(uint64_t type) {
    static struct ct_asset_preview_i0 ct_asset_preview_i0 = {
            .load = load,
    };

    if (type == ASSET_PREVIEW_I0) {
        return &ct_asset_preview_i0;
    };

    return NULL;
}

static const char *display_icon() {
    return ICON_FA_CUBES;
}

static const char *name() {
    return "entity";
}

static struct ct_asset_i0 ct_asset_api = {
        .name = name,
        .cdb_type = cdb_type,
        .display_icon = display_icon,
        .get_interface = asset_get_interface,
};

static bool supported_extension(const char *extension) {
    return !strcmp(extension, "entity");
}

static ct_asset_io_i0 entitiy_io = {
        .supported_extension = supported_extension,
};

//==============================================================================
// Public interface
//==============================================================================

static bool alive(ct_world_t0 world,
                  struct ct_entity_t0 entity) {
    world_instance_t *w = get_world_instance(world);
    return ce_handler_alive(&w->entity_handler, entity.h);
}

static struct ct_entity_t0 spawn_entity(ct_world_t0 world,
                                        uint64_t name) {
    if (!name) {
        return (ct_entity_t0) {0};
    }

    ce_cdb_t0 db = ce_cdb_a0->db();

    uint64_t entity_obj = name;
    const ce_cdb_obj_o0 *ent_reader = ce_cdb_a0->read(db, entity_obj);
    if (!ent_reader) {
        return (ct_entity_t0) {0};
    }

    world_instance_t *w = get_world_instance(world);

    ct_entity_t0 root_ent;
    create_entities_objs(world, &root_ent, 1, &entity_obj);

    uint64_t components_n = ce_cdb_a0->read_objset_num(ent_reader, ENTITY_COMPONENTS);
    uint64_t components_keys[components_n];
    ce_cdb_a0->read_objset(ent_reader, ENTITY_COMPONENTS, components_keys);

    ct_archemask_t0 ent_type = combine_component_obj(db, components_keys, components_n);

    _add_ent_spawn_obj(db, w, entity_obj, root_ent);

    _add_components_to_archetype(world, root_ent, ent_type);

    archetype_t *storage = _get_archetype(w, ent_type);

    const uint64_t idx = _entity_data_idx(w, root_ent);
    ent_chunk_t *chunk = _entity_chunk(w, root_ent);

    for (int i = 0; i < components_n; ++i) {
        uint64_t component_obj = components_keys[i];
        uint64_t component_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), component_obj);
        uint64_t cidx = component_idx(storage, component_type);

        ct_ecs_component_i0 *ci = get_component_interface(component_type);

        if (!ci) {
            continue;
        }

        uint32_t size = storage->size[cidx];

        if (size) {
            void *data = _get_component_array(storage, chunk, cidx);

            uint8_t *comp_data = data + (idx * size);

            if (ci->from_cdb_obj) {
                ci->from_cdb_obj(world, ce_cdb_a0->db(), component_obj, comp_data);
            }
        }

        _add_comp_spawn_obj(db, w, component_obj, root_ent);
    }

    uint64_t children_n = ce_cdb_a0->read_objset_num(ent_reader, ENTITY_CHILDREN);
    uint64_t children[children_n];
    ce_cdb_a0->read_objset(ent_reader, ENTITY_CHILDREN, children);

    for (int i = 0; i < children_n; ++i) {
        uint64_t child = children[i];
        ct_entity_t0 child_ent = spawn_entity(world, child);

        add_components(world, child_ent,
                       CE_ARR_ARG(((ct_component_pair_t0[]) {
                               {
                                       .type = CT_PARENT_COMPONENT,
                                       .data = &(ct_parent_c) {
                                               .parent = root_ent,
                                       }
                               }
                       })));
    }

//    ce_cdb_a0->log_obj(LOG_WHERE, ce_cdb_a0->db(), entity_obj);

    return root_ent;
}

//==============================================================================
// Public interface
//==============================================================================
static struct world_instance_t *_new_world(ct_world_t0 world) {
    uint32_t idx = ce_array_size(_G.world_array);

    world_instance_t wi = {
            .entity_idx  = virtual_alloc(sizeof(uint64_t) * MAX_ENTITIES),
            .entity_chunk = virtual_alloc(sizeof(ent_chunk_t *) * MAX_ENTITIES),
    };

    _init_spawn_infos(&wi.obj_spawninfo);
    _init_spawn_infos(&wi.comp_spawninfo);

    ce_array_push(_G.world_array, wi, _G.allocator);

    ce_handler_create(&_G.world_array[idx].entity_handler, _G.allocator);

    ce_hash_add(&_G.world_map, world.h, idx, _G.allocator);
    return &_G.world_array[idx];
}

static ct_world_t0 create_world(const char *name) {
    ct_world_t0 world = {.h = ce_handler_create(&_G.world_handler, _G.allocator)};

    CE_ASSERT("ecs", world.h != 0);
    ce_array_push(_G.worlds, world, _G.allocator);

    world_instance_t *w = _new_world(world);

    w->world = world;
    w->db = ce_cdb_a0->db();
    w->global_world_version = 1;
    w->name = ce_memory_a0->str_dup(name, _G.allocator);


    return world;
}

static void destroy_world(ct_world_t0 world) {
    ce_handler_destroy(&_G.world_handler, world.h, _G.allocator);
}

static void query_collect_ents(ct_world_t0 world,
                               ct_ecs_query_t0 query,
                               ct_entity_t0 **ents,
                               const ce_alloc_t0 *alloc) {
    world_instance_t *w = get_world_instance(world);

    const uint32_t type_count = ce_array_size(w->archetype_array);

    for (int i = 0; i < type_count; ++i) {
        archetype_t *storage = &w->archetype_pool[w->archetype_array[i]];

        ent_chunk_t *chunk = storage->first;

        if (!_can_run_query_on_archetype(storage->archetype_mask, &query)) {
            continue;
        }

        ce_array_push_n(*ents, ents, chunk->ent_n, alloc);
    }
}

static ct_entity_t0 query_get_first(ct_world_t0 world,
                                    ct_ecs_query_t0 query) {
    world_instance_t *w = get_world_instance(world);

    const uint32_t type_count = ce_array_size(w->archetype_pool);

    for (int i = 0; i < type_count; ++i) {
        archetype_t *storage = &w->archetype_pool[i];

        ent_chunk_t *chunk = storage->first;

        if (!_can_run_query_on_archetype(storage->archetype_mask, &query)) {
            continue;
        }

        ct_entity_t0 *ents = _get_entity_array(chunk);
        return ents[0];
    }

    return (ct_entity_t0) {0};
}

static struct ct_ecs_a0 _api = {
        // WORLD
        .create_world = create_world,
        .destroy_world = destroy_world,
        .step = step,

        .buff_add_component = add_buff,
        .buff_remove_component = remove_buff,

};


struct ct_ecs_a0 *ct_ecs_a0 = &_api;

static struct ct_ecs_e_a0 e_api = {
        // ENTITY
        .create_entities = create_entities,
        .destroy_entities = destroy,
        .entity_alive = alive,
        .spawn_entity = spawn_entity,
};

struct ct_ecs_e_a0 *ct_ecs_e_a0 = &e_api;

static struct ct_ecs_c_a0 c_api = {
        .has = has,
        .get_interface = get_component_interface,
        .combine_component = combine_component,
        .get_all = get_all,
        .get_one = get_one,
        .add = add_components,
        .remove = remove_components,
};
struct ct_ecs_c_a0 *ct_ecs_c_a0 = &c_api;

static struct ct_ecs_q_a0 q_api = {
        .collect_ents = query_collect_ents,
        .first = query_get_first,
        .foreach = process_query,
        .foreach_serial = process_query_serial,
};
struct ct_ecs_q_a0 *ct_ecs_q_a0 = &q_api;


static void _on_api_add(uint64_t name,
                        void *api) {

    if (CT_ECS_COMPONENT_I0 == name) {
        ct_ecs_component_i0 *component_i = api;

        if (!component_i->cdb_type) {
            ce_log_a0->error(LOG_WHERE, "component does not implement cdb_type()");
            return;
        }

        bool contian = false;
        for (int i = 0; i < ce_array_size(_G.components_name); ++i) {
            if (_G.components_name[i] != name) {
                continue;
            }
            contian = true;
            break;
        }

        ce_hash_add(&_G.component_interface_map, component_i->cdb_type,
                    (uint64_t) component_i, _G.allocator);

        if (!contian) {
            const uint64_t cid = _G.component_count++;
            ce_hash_add(&_G.component_types, component_i->cdb_type, cid, _G.allocator);
            ce_array_push(_G.components_name, component_i->cdb_type, _G.allocator);
            if (component_i->is_system_state) {
                _G.system_state_components_mask |= (uint64_t) (1llu << cid);
            }
        }

    } else if (CT_ECS_SYSTEM_I0 == name) {
        uint32_t n = ce_array_size(_G.world_array);
        for (int i = 0; i < n; ++i) {
            _G.world_array[i].global_world_version++;
        }
    }
}


static void _sync_task(float dt) {
//    return;
    uint32_t wn = ce_array_size(_G.world_array);
    ct_entity_t0 *ents = NULL;

    ce_cdb_t0 db = ce_cdb_a0->db();

    // changed
    uint32_t obj_ev_n = 0;
    const ce_cdb_ev_t0 *obj_ev = ce_cdb_a0->changes(ce_cdb_a0->db(), &obj_ev_n);

    for (uint32_t i = 0; i < obj_ev_n; ++i) {
        ce_cdb_ev_t0 objs_ev = obj_ev[i];

        if (objs_ev.ev_type == CE_CDB_OBJ_DESTROY_EVENT) {
            uint64_t obj = objs_ev.obj;
            uint64_t type = objs_ev.obj_type;

            if (type == ENTITY_INSTANCE) {
                for (uint32_t i = 0; i < wn; ++i) {
                    struct world_instance_t *world = &_G.world_array[i];

                    spawn_info_t *si = _get_spawninfo(&world->obj_spawninfo, obj);

                    if (!si) {
                        continue;
                    }

                    destroy(world->world, si->ents, ce_array_size(si->ents));
                    _free_spawninfo(&world->obj_spawninfo, obj);
                }
            } else {
                if (!get_component_interface(objs_ev.obj_type)) {
                    continue;
                }

                for (uint32_t i = 0; i < wn; ++i) {
                    struct world_instance_t *world = &_G.world_array[i];

                    uint64_t parent = ce_cdb_a0->parent(db, obj);
                    uint64_t parent_type = ce_cdb_a0->obj_type(db, parent);

                    if (parent_type != ENTITY_INSTANCE) {
                        continue;
                    }

                    spawn_info_t *si = _get_spawninfo(&world->obj_spawninfo, parent);

                    if (!si) {
                        continue;
                    }

                    uint32_t ents_n = ce_array_size(si->ents);
                    for (int e = 0; e < ents_n; ++e) {
                        ct_entity_t0 ent = si->ents[e];
                        remove_components(world->world, ent, &type, 1);
                    }
                }
            }
        }
    }

    uint32_t objs_ev_n = 0;
    const ce_cdb_prop_ev_t0 *objs_ev = ce_cdb_a0->objs_changes(ce_cdb_a0->db(), &objs_ev_n);

    for (uint32_t ev_i = 0; ev_i < objs_ev_n; ++ev_i) {
        ce_cdb_prop_ev_t0 ev = objs_ev[ev_i];

        uint64_t type = ce_cdb_a0->obj_type(db, ev.obj);
        // is entity?
        if (type == ENTITY_INSTANCE) {
            for (uint32_t i = 0; i < wn; ++i) {
                struct world_instance_t *world = &_G.world_array[i];
                spawn_info_t *si = _get_spawninfo(&world->obj_spawninfo, ev.obj);

                if (!si) {
                    continue;
                }

                if (ev.prop == ENTITY_CHILDREN) {
                    if (ev.ev_type == CE_CDB_OBJSET_ADD_EVENT) {
                        uint64_t ent_obj = ev.new_value.subobj;

                        uint64_t ents_n = ce_array_size(si->ents);
                        for (int e = 0; e < ents_n; ++e) {
                            ct_entity_t0 ent = si->ents[e];
                            ct_entity_t0 new_ents = spawn_entity(world->world, ent_obj);

                            add_components(world->world, new_ents,
                                           CE_ARR_ARG(((ct_component_pair_t0[]) {
                                                   {
                                                           .type = CT_PARENT_COMPONENT,
                                                           .data = &(ct_parent_c) {
                                                                   .parent = ent,
                                                           }
                                                   }
                                           })));

                        }

                    } else if (ev.ev_type == CE_CDB_PROP_MOVE_EVENT) {
                        uint64_t ent_obj = ev.value.subobj;
                        uint64_t to_ent_obj = ev.to;

                        spawn_info_t *to_si = _get_spawninfo(&world->obj_spawninfo,
                                                             to_ent_obj);
                        if (!to_si) {
                            continue;
                        }

                        spawn_info_t *ent_si = _get_spawninfo(&world->obj_spawninfo,
                                                              ent_obj);
                        if (!ent_si) {
                            continue;
                        }

                        for (int j = 0; j < ce_array_size(to_si->ents); ++j) {
                            ct_entity_t0 to_ent = to_si->ents[j];
                            ct_entity_t0 ent = ent_si->ents[j];

                            add_components(world->world, ent,
                                           CE_ARR_ARG(((ct_component_pair_t0[]) {
                                                   {
                                                           .type = CT_PARENT_COMPONENT,
                                                           .data = &(ct_parent_c) {
                                                                   .parent = to_ent,
                                                           }
                                                   }
                                           })));
                        }
                    }
                } else if (ev.prop == ENTITY_COMPONENTS) {
                    if (ev.ev_type == CE_CDB_OBJSET_ADD_EVENT) {
                        uint64_t comp_obj = ev.new_value.subobj;
                        uint64_t k = ce_cdb_a0->obj_type(db, comp_obj);

                        uint64_t ents_n = ce_array_size(si->ents);
                        for (int e = 0; e < ents_n; ++e) {
                            ct_entity_t0 ent = si->ents[e];
                            ct_archemask_t0 ent_type = _entity_chunk(world,
                                                                     ent)->archetype_mask;

                            ct_archemask_t0 new_type = combine_component(&k, 1);
                            new_type = _archetype_add(ent_type, new_type);

                            if (_archetype_eq(ent_type, new_type)) {
                                continue;
                            }

                             _add_components_from_obj(world, db, ent, comp_obj);
                        }
                    }
                }
            }
        } else {
            for (uint32_t i = 0; i < wn; ++i) {
                struct world_instance_t *world = &_G.world_array[i];

                spawn_info_t *si = _get_component_spawninfo(world, ev.obj);

                if (!si) {
                    continue;
                }

                uint64_t comp_type = ce_cdb_a0->obj_type(db, si->ent_obj);

                ct_ecs_component_i0 *ci = get_component_interface(comp_type);

                if (!ci) {
                    continue;
                }

                uint64_t ents_n = ce_array_size(si->ents);
                for (int e = 0; e < ents_n; ++e) {
                    ct_entity_t0 ent = si->ents[e];

                    void *data = get_one(world->world, comp_type, ent, true);

                    if (!data) {
                        continue;
                    }

                    if (ci->from_cdb_obj) {
                        ci->from_cdb_obj(world->world, db, si->ent_obj, data);
                    }
                }
            }
        }
    }

    ce_array_free(ents, _G.allocator);
}

static struct ct_kernel_task_i0 ecs_sync_task = {
        .name = CT_ECS_SYNC_TASK,
        .update = _sync_task,
        .update_after = CT_KERNEL_AFTER(CT_EDITOR_TASK),
        .update_before = CT_KERNEL_BEFORE(CT_GAME_TASK),
};

static ce_cdb_prop_def_t0 entity_prop[] = {
        {
                .name = "name",
                .type = CE_CDB_TYPE_STR,
        },
        {
                .name = "components",
                .type = CE_CDB_TYPE_SET_SUBOBJECT,
                .obj_type = ENTITY_COMPONENTS,
        },
        {
                .name = "children",
                .type = CE_CDB_TYPE_SET_SUBOBJECT,
                .obj_type = ENTITY_CHILDREN,
        },
};


////

static ct_system_group_i0 simulation_sysg = {
        .name = CT_ECS_SIMULATION_GROUP,
};

static ct_system_group_i0 presentation_sysg = {
        .name = CT_ECS_PRESENTATION_GROUP,
};

////
#define CT_ECS_DEBUG_DOCK  \
    CE_ID64_0("ecs_debug", 0x7437a9dcca9e36c4ULL)

typedef struct ecs_debugger_t {
    ct_world_t0 w;
} ecs_debugger_t;
static ecs_debugger_t ecs_dbg;

static const char *ecs_debuger_title() {
    return "ECS debugger";
}

static const char *ecs_debuger_name(uint64_t dock) {
    return "ecs_debuger_dock";
}

static void ecs_debuger_draw(uint64_t content,
                             uint64_t context,
                             uint64_t selected_object) {
    int cur_item = 0;
    const char **worlds_str = NULL;
    for (int i = 0; i < ce_array_size(_G.world_array); ++i) {
        ct_world_t0 w = _G.world_array[i].world;

        if (ecs_dbg.w.h == w.h) {
            cur_item = i;
        }

        world_instance_t *wi = get_world_instance(w);
        ce_array_push(worlds_str, wi->name, _G.allocator);
    }

    if (ct_debugui_a0->Combo("world", &cur_item, worlds_str, ce_array_size(worlds_str), -1)) {
        ecs_dbg.w = _G.world_array[cur_item].world;
    }

    world_instance_t *w = get_world_instance(ecs_dbg.w);

    if (w) {
        char *str_buff = NULL;

        for (int i = 0; i < ce_array_size(w->archetype_array); ++i) {
            archetype_t *archetype = &w->archetype_pool[w->archetype_array[i]];

            for (int j = 0; j < archetype->component_n; ++j) {
                uint64_t component_name = archetype->name[j];
                ct_ecs_component_i0 *ci = get_component_interface(component_name);
                const char *display_name = ci->display_name ? ci->display_name() : NULL;
                ce_buffer_printf(&str_buff, _G.allocator, "%s | ", display_name);
            }

            if (archetype->archetype_mask.mask == 0) {
                ce_buffer_printf(&str_buff, _G.allocator, "empty");
            }

            ct_debugui_a0->Selectable(str_buff, false, 0, &CE_VEC2_ZERO);
            ce_array_clean(str_buff);
        }

        ce_array_free(str_buff, _G.allocator);
    }

    ce_array_free(worlds_str, _G.allocator);
}

static struct ct_dock_i0 ecs_debuger_dock = {
        .type = CT_ECS_DEBUG_DOCK,
        .display_title = ecs_debuger_title,
        .name = ecs_debuger_name,
        .draw_ui = ecs_debuger_draw,
};
////

void CE_MODULE_LOAD(ecs)(struct ce_api_a0 *api,
                         int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_asset_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ce_task_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .db = ce_cdb_a0->db(),
    };

    ce_id_a0->id64("entity");

    ce_handler_create(&_G.world_handler, _G.allocator);

    ce_array_set_capacity(_G.cmd_buf_pool, 64, _G.allocator);

    api->add_api(CT_ECS_A0_STR, &_api, sizeof(_api));
    api->add_api(CT_ECS_E_A0_STR, &e_api, sizeof(e_api));
    api->add_api(CT_ECS_C_A0_STR, &c_api, sizeof(c_api));
    api->add_api(CT_ECS_Q_A0_STR, &q_api, sizeof(q_api));

    api->add_impl(CT_ASSET_I0_STR, &ct_asset_api, sizeof(ct_asset_api));
    api->add_impl(CT_ASSET_IO_I0_STR, &entitiy_io, sizeof(entitiy_io));
    api->add_impl(CT_KERNEL_TASK_I0_STR, &ecs_sync_task, sizeof(ecs_sync_task));

    api->add_impl(CT_ECS_SYSTEM_GROUP_I0_STR, &simulation_sysg, sizeof(simulation_sysg));
    api->add_impl(CT_ECS_SYSTEM_GROUP_I0_STR, &presentation_sysg, sizeof(presentation_sysg));

    api->add_impl(CT_DOCK_I0_STR, &ecs_debuger_dock, sizeof(ecs_debuger_dock));

    api->register_on_add(_on_api_add);

    ce_cdb_a0->reg_obj_type(ENTITY_INSTANCE, entity_prop, CE_ARRAY_LEN(entity_prop));
}

void CE_MODULE_UNLOAD(ecs)(struct ce_api_a0 *api,
                           int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}