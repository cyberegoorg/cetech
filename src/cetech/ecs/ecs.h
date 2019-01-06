#ifndef CETECH_ECS_H
#define CETECH_ECS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CT_ECS_API \
    CE_ID64_0("ct_ecs_a0", 0x2abf54879e52240fULL)

#define CT_ECS_SYNC_TASK \
    CE_ID64_0("ecs_sync_task", 0x5dd474f338cddf7ULL)

#define EDITOR_COMPONENT \
    CE_ID64_0("ct_editor_component_i0", 0x5b3beb29b490cfd8ULL)

#define COMPONENT_I \
    CE_ID64_0("ct_component_i0", 0x3a1ad5e3ea21da79ULL)

#define ENTITY_INSTANCE \
    CE_ID64_0("entity", 0x9831ca893b0d087dULL)

#define ENTITY_NAME \
    CE_ID64_0("name", 0xd4c943cba60c270bULL)

#define ENTITY_COMPONENTS \
    CE_ID64_0("components", 0xe71d1687374e5a54ULL)

#define ENTITY_CHILDREN \
    CE_ID64_0("children", 0x6fbb13de0e1dce0dULL)

#define ENTITY_RESOURCE_ID \
    CE_ID64_0("entity", 0x9831ca893b0d087dULL)

#define SIMULATION_INTERFACE \
    CE_ID64_0("ct_simulation_i0", 0xe944056f6e473ecdULL)

#define COMPONENT_INTERFACE \
    CE_ID64_0("ct_component_i0", 0x3a1ad5e3ea21da79ULL)

struct ct_cdb_obj_t;
enum ce_cdb_type;
typedef void ce_cdb_obj_o;
typedef void ct_entity_storage_t;
struct ct_resource_id;

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

struct ct_component_i0 {
    uint64_t (*cdb_type)();

    void *(*get_interface)(uint64_t name_hash);

    void (*spawner)(struct ct_world world,
                    uint64_t obj);

    void (*obj_change)(struct ct_world world,
                       uint64_t obj,
                       const uint64_t *prop,
                       uint32_t prop_count,
                       struct ct_entity *ents,
                       uint32_t n);
};


struct ct_editor_component_i0 {
    const char *(*display_name)();

    uint64_t (*create_new)();

    void (*guizmo_get_transform)(uint64_t obj,
                                 float *world,
                                 float *local);

    void (*guizmo_set_transform)(uint64_t obj,
                                 uint8_t operation,
                                 float *world,
                                 float *local);
};

struct ct_simulation_i0 {
    uint64_t (*name)();

    const uint64_t *(*before)(uint32_t *n);

    const uint64_t *(*after)(uint32_t *n);

    ct_simulate_fce_t simulation;
};


struct ct_ecs_a0 {
    //WORLD
    struct ct_world (*create_world)();

    //ENT
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


    //SIMU
    void (*simulate)(struct ct_world world,
                     float dt);

    void (*process)(struct ct_world world,
                    uint64_t components_mask,
                    ct_process_fce_t fce,
                    void *data);

    //COMP
    struct ct_component_i0 *(*get_interface)(uint64_t name);

    uint64_t (*mask)(uint64_t component_name);

    uint64_t *(*get_all)(uint64_t component_name,
                         ct_entity_storage_t *item);

    uint64_t (*get_one)(struct ct_world world,
                        uint64_t component_name,
                        struct ct_entity entity);

    void (*add)(struct ct_world world,
                struct ct_entity ent,
                const uint64_t *component_name,
                uint32_t name_count,
                uint64_t *data);

    void (*remove)(struct ct_world world,
                   struct ct_entity ent,
                   const uint64_t *component_name,
                   uint32_t name_count);
};

CE_MODULE(ct_ecs_a0);

#endif //CETECH_ECS_H