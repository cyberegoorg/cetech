#ifndef CETECH_ECS_H
#define CETECH_ECS_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define ECS_EBUS_NAME "ecs"

#define PREFAB_NAME_PROP CT_ID64_0("prefab_filename")
#define EDITOR_COMPONENT CT_ID64_0("ct_editor_component_i0")

enum {
    ECS_EBUS = 0x3c870dac
};

enum {
    ECS_INVALID_EVENT = 0,
    ECS_WORLD_CREATE,
    ECS_WORLD_DESTROY,

    ECS_COMPONENT_REMOVE,
    ECS_COMPONENT_SPAWN,
    ECS_COMPONENT_COMPILE,
};

struct ct_cdb_obj_t;
typedef void ct_entity_storage_t;

//==============================================================================
// Enums
//==============================================================================
enum ct_cdb_type;

//==============================================================================
// Typedefs
//==============================================================================

struct ct_world {
    uint64_t h;
};

struct ct_entity {
    uint64_t h;
};

struct ct_component_info {
    const char *component_name;
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

struct ct_component_i0 {
    uint64_t (*cdb_type)();

    void *(*get_interface)(uint64_t name_hash);
};

struct ct_editor_component_i0 {
    const char *(*display_name)();

    void (*property_editor)(uint64_t obj);

    void (*gizmo_get_transform)(uint64_t obj, float* world, float* local);
    void (*gizmo_set_transform)(uint64_t obj, float* world, float* local);
};

struct ct_ecs_a0 {
    // WORLD
    struct ct_world (*create_world)();

    void (*destroy_world)(struct ct_world world);

    // ENT
    uint64_t (*entity_object)(struct ct_world world,
                              struct ct_entity entity);

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

    const uint64_t *(*get_components)();

    struct ct_component_info *(*component_info)(uint64_t component_name);

    uint64_t (*component_mask)(uint64_t component_name);

    void *(*entities_data)(uint64_t component_name,
                           ct_entity_storage_t *item);

    void *(*entity_data)(struct ct_world world,
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

CT_MODULE(ct_ecs_a0);

#endif //CETECH_ECS_H