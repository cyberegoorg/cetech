#ifndef CETECH_ECS_H
#define CETECH_ECS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define ECS_EBUS_NAME "ecs"

#define PREFAB_NAME_PROP \
    CE_ID64_0("prefab_filename", 0x74e7f49a67b7125fULL)

#define EDITOR_COMPONENT \
    CE_ID64_0("ct_editor_component_i0", 0x5b3beb29b490cfd8ULL)

#define COMPONENT_I \
    CE_ID64_0("ct_component_i0", 0x3a1ad5e3ea21da79ULL)

#define ENTITY_INSTANCE \
    CE_ID64_0("entity", 0x9831ca893b0d087dULL)

#define ENTITY_RESOURCE \
    CE_ID64_0("entity_resource", 0xf8623393c111abd5ULL)

#define ENTITY_TYPE \
    CE_ID64_0("entity_type", 0x61c464fe6c327f9fULL)

#define ENTITY_DATA_IDX  \
    CE_ID64_0("entity_data_idx", 0x1fa649061369a0eeULL)

#define ENTITY_UID \
    CE_ID64_0("entity_uid", 0xa3b266878c572abdULL)

#define ENTITY_NAME \
    CE_ID64_0("name", 0xd4c943cba60c270bULL)

#define ENTITY_WORLD \
    CE_ID64_0("world", 0x4d46ae3bbc0fb0f7ULL)

#define ENTITY_COMPONENTS \
    CE_ID64_0("components", 0xe71d1687374e5a54ULL)

#define ENTITY_CHILDREN \
    CE_ID64_0("children", 0x6fbb13de0e1dce0dULL)

#define ENTITY_RESOURCE_ID \
    CE_ID64_0("entity", 0x9831ca893b0d087dULL)


enum {
    ECS_EBUS = 0x3c870dac
};

enum {
    ECS_INVALID_EVENT = 0,
    ECS_WORLD_CREATE,
    ECS_WORLD_DESTROY,
};

struct ct_cdb_obj_t;
typedef void ct_entity_storage_t;

struct ct_world {
    uint64_t h;
};

struct ct_entity {
    uint64_t h;
};

typedef void (*ct_process_fce_t)(struct ct_world world,
                                 struct ct_entity *ent,
                                 ct_entity_storage_t *item,
                                 uint32_t n,
                                 void *data);

typedef void (*ct_simulate_fce_t)(struct ct_world world,
                                  float dt);

enum ce_cdb_type;
typedef void ce_cdb_obj_o;

#define COMPONENT_INTERFACE_NAME \
    "ct_component_i0"

#define COMPONENT_INTERFACE \
    CE_ID64_0("ct_component_i0", 0x3a1ad5e3ea21da79ULL)

struct ct_component_prop_desc {
    uint32_t offset;
    uint64_t prop;
    uint64_t cdb_type;
};

struct ct_component_i0 {
    uint64_t (*size)();

    const struct ct_component_prop_desc* (*prop_desc)(uint32_t* count);

    uint64_t (*cdb_type)();

    void *(*get_interface)(uint64_t name_hash);

    void (*compiler)(const char *filename,
                     uint64_t *component_key,
                     uint32_t component_key_count,
                     ce_cdb_obj_o *writer);

    void (*spawner)(uint64_t obj,
                    void *data);
};

struct ct_editor_component_i0 {
    const char *(*display_name)();

    void (*property_editor)(uint64_t obj);

    void (*guizmo_get_transform)(uint64_t obj,
                                 float *world,
                                 float *local);

    void (*guizmo_set_transform)(uint64_t obj,
                                 uint8_t operation,
                                 float *world,
                                 float *local);
};

struct ct_component_a0 {
    struct ct_component_i0 *(*get_interface)(uint64_t name);

    uint64_t (*mask)(uint64_t component_name);

    void *(*get_all)(uint64_t component_name,
                     ct_entity_storage_t *item);

    void *(*get_one)(struct ct_world world,
                     uint64_t component_name,
                     struct ct_entity entity);

    void (*add)(struct ct_world world,
                struct ct_entity ent,
                uint64_t *component_name,
                uint32_t name_count);

    void (*remove)(struct ct_world world,
                   struct ct_entity ent,
                   uint64_t *component_name,
                   uint32_t name_count);
};

struct ct_entity_a0 {
    struct ct_world (*create_world)();

    void (*destroy_world)(struct ct_world world);

    void (*create)(struct ct_world world,
                   struct ct_entity *entity,
                   uint32_t count);

    void (*destroy)(struct ct_world world,
                    struct ct_entity *entity,
                    uint32_t count);

    bool (*alive)(struct ct_world world,
                  struct ct_entity entity);

    struct ct_entity (*spawn)(struct ct_world world,
                              uint64_t name);


    bool (*has)(struct ct_world world,
                struct ct_entity ent,
                uint64_t *component_name,
                uint32_t name_count);

    struct ct_entity (*find_by_name)(struct ct_world world,
                                     struct ct_entity ent, uint64_t name);
};

struct ct_system_a0 {
    void (*simulate)(struct ct_world world,
                     float dt);

    void (*process)(struct ct_world world,
                    uint64_t components_mask,
                    ct_process_fce_t fce,
                    void *data);

    void (*register_simulation)(const char *name,
                                ct_simulate_fce_t simulation);
};

struct ct_ecs_a0 {
    struct ct_entity_a0 *entity;
    struct ct_component_a0 *component;
    struct ct_system_a0 *system;
};

CE_MODULE(ct_ecs_a0);

#endif //CETECH_ECS_H