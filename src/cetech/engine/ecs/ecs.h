#ifndef CETECH_WORLD_H
#define CETECH_WORLD_H


#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define ECS_EBUS_NAME "ecs"

enum {
    ECS_EBUS = 0x3c870dac
};

enum {
    ECS_INVALID_EVENT = 0,
    ECS_WORLD_CREATE,
    ECS_WORLD_DESTROY,

    ECS_COMPONENT_ADD,
    ECS_COMPONENT_REMOVE,
    ECS_COMPONENT_CHANGE,
    ECS_COMPONENT_SPAWN,
    ECS_COMPONENT_COMPILE,
};

struct ct_entity_compile_output;
struct ct_compilator_api;
struct ct_cdb_obj_t;
typedef void ct_entity_storage_t;

//==============================================================================
// Enums
//==============================================================================


//==============================================================================
// Typedefs
//==============================================================================

struct ct_world {
    uint64_t h;
};

struct ct_entity {
    uint64_t h;
};

struct ct_ecs_world_ev {
    struct ct_world world;
};

struct ct_ecs_component_ev {
    struct ct_world world;
    struct ct_entity ent;
    uint64_t comp_mask;
};

struct ct_ecs_component_spawn_ev {
    struct ct_cdb_obj_t *obj;
    void *data;
};

struct ct_ecs_component_compile_ev {
    const char *filename;
    uint64_t *component_key;
    uint32_t component_key_count;
    struct ct_cdb_obj_t *writer;
};

struct ct_component_prop_map {
    uint64_t key;
    uint64_t offset;
};

struct ct_component_info {
    const char *component_name;
    uint64_t component_name_hash;
    struct ct_component_prop_map *prop_map;
    uint32_t prop_count;
    uint64_t size;
};

//==============================================================================
// Structs
//==============================================================================

typedef void (*ct_process_fce_t)(struct ct_world world,
                                 struct ct_entity *ent,
                                 ct_entity_storage_t *item,
                                 uint32_t n,
                                 void *data);

typedef void (*ct_simulate_fce_t)(struct ct_world world,
                                  float dt);

//==============================================================================
// Api
//==============================================================================


struct ct_ecs_a0 {
    // WORLD
    struct ct_world (*create_world)();

    void (*destroy_world)(struct ct_world world);

    // ENT
    void (*create_entity)(struct ct_world world,
                          struct ct_entity *entity,
                          uint32_t count);

    void (*destroy_entity)(struct ct_world world,
                           struct ct_entity *entity,
                           uint32_t count);

    bool (*entity_alive)(struct ct_world world,
                         struct ct_entity entity);

    struct ct_entity (*spawn_entity)(struct ct_world world,
                                     uint32_t name);

    struct ct_entity (*find_by_uid)(struct ct_world world,
                                    struct ct_entity root,
                                    uint64_t uid);

    void (*link)(struct ct_world world,
                 struct ct_entity parent,
                 struct ct_entity child);

    // COMPONENT
    void (*register_component)(struct ct_component_info info);

    uint64_t (*component_mask)(uint64_t component_name);

    void *(*component_data)(uint64_t component_name,
                            ct_entity_storage_t *item);

    void *(*entity_data)(struct ct_world world,
                         uint64_t component_name,
                         struct ct_entity entity);


    void (*entity_component_change)(struct ct_world world,
                                    uint64_t component_name,
                                    struct ct_entity entity);

    bool (*has)(struct ct_world world,
                struct ct_entity ent,
                uint64_t *component_name,
                uint32_t name_count);

    void (*add_components)(struct ct_world world,
                           struct ct_entity ent,
                           uint64_t *component_name,
                           uint32_t name_count);

    void (*remove_components)(struct ct_world world,
                              struct ct_entity ent,
                              uint64_t *component_name,
                              uint32_t name_count);

    void (*simulate)(struct ct_world world,
                     float dt);

    void (*process)(struct ct_world world,
                    uint64_t components_mask,
                    ct_process_fce_t fce,
                    void *data);

    void (*register_simulation)(const char *name,
                                ct_simulate_fce_t simulation);

//    void (*add_simulation)(struct ct_world world,
//                           uint64_t name);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_WORLD_H
//! \}