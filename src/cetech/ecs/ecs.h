#ifndef CETECH_ECS_H
#define CETECH_ECS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CT_ECS_API \
    CE_ID64_0("ct_ecs_a0", 0x2abf54879e52240fULL)

#define CT_ECS_E_API \
    CE_ID64_0("ct_ecs_e_a0", 0x16082519819f93abULL)

#define CT_ECS_C_API \
    CE_ID64_0("ct_ecs_c_a0", 0xfef0f9e05beb0918ULL)

#define CT_ECS_Q_API \
    CE_ID64_0("ct_ecs_q_a0", 0x6335c7d4eda844edULL)

#define CT_ECS_SYNC_TASK \
    CE_ID64_0("ecs_sync_task", 0x5dd474f338cddf7ULL)

#define CT_EDITOR_COMPONENT_I \
    CE_ID64_0("ct_editor_component_i0", 0x5b3beb29b490cfd8ULL)

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

#define CT_ECS_SYSTEM_I \
    CE_ID64_0("ct_system_i0", 0x1b993a5859f15c5cULL)

#define CT_ECS_SYSTEM_GROUP_I \
    CE_ID64_0("ct_system_group_i0", 0xabe09ba0753e7968ULL)

#define CT_ECS_COMPONENT_I \
    CE_ID64_0("ct_ecs_component_i0", 0x9fda2263be703cb3ULL)

#define CT_ECS_SIMULATION_GROUP \
    CE_ID64_0("simulation_group", 0xf085d4e5a160c140ULL)

#define CT_ECS_PRESENTATION_GROUP \
    CE_ID64_0("presentation_group", 0xfba87d270fbfa0b1ULL)

#define _ECS_LIST(...) (uint64_t[]){__VA_ARGS__}
#define _ECS_LIST_SIZE(list) (sizeof(list) / sizeof(list[0]))

#define CT_ECS_BEFORE(...) \
    ((ce_ptr_pair_t0){.ptr=&_ECS_LIST(__VA_ARGS__),.len=_ECS_LIST_SIZE(_ECS_LIST(__VA_ARGS__))})

#define CT_ECS_AFTER(...) \
    ((ce_ptr_pair_t0){.ptr=&_ECS_LIST(__VA_ARGS__),.len=_ECS_LIST_SIZE(_ECS_LIST(__VA_ARGS__))})

#define CT_ECS_ARCHETYPE(...) \
    ct_ecs_c_a0->combine_component(_ECS_LIST(__VA_ARGS__), _ECS_LIST_SIZE(_ECS_LIST(__VA_ARGS__)))


typedef struct ce_cdb_t0 ce_cdb_t0;

typedef struct ct_ecs_ent_chunk_o0 ct_ecs_ent_chunk_o0;
typedef struct ct_ecs_cmd_buffer_t ct_ecs_cmd_buffer_t;

typedef struct ct_world_t0 {
    uint64_t h;
} ct_world_t0;

typedef struct ct_entity_t0 {
    uint64_t h;
} ct_entity_t0;

typedef struct ct_archetype_mask_t0 {
    uint64_t mask;
} ct_archemask_t0;

typedef struct ct_ecs_component_i0 {
    bool is_system_state;
    uint64_t cdb_type;
    uint64_t size;

    const char *(*display_name)();

    void (*from_cdb_obj)(ct_world_t0 world,
                         ce_cdb_t0 db,
                         uint64_t obj,
                         void *data);

    void *(*get_interface)(uint64_t name_hash);
} ct_ecs_component_i0;

typedef struct ct_component_pair_t0 {
    uint64_t type;
    void *data;
} ct_component_pair_t0;

typedef struct ct_ecs_query_t0 {
    ct_archemask_t0 all;
    ct_archemask_t0 any;
    ct_archemask_t0 none;
    ct_archemask_t0 write;
    bool only_changed;
} ct_ecs_query_t0;

typedef void (*ct_ecs_foreach_fce_t)(ct_world_t0 world,
                                     ct_entity_t0 *ent,
                                     ct_ecs_ent_chunk_o0 *chunk,
                                     uint32_t n,
                                     void *data);

typedef void (ct_ecs_system_fce_t)(ct_world_t0 world,
                                   float dt,
                                   uint32_t rq_version,
                                   ct_ecs_cmd_buffer_t *cmd);


typedef struct ct_editor_component_i0 {
    void (*guizmo_get_transform)(uint64_t obj,
                                 float *world,
                                 float *local);

    void (*guizmo_set_transform)(uint64_t obj,
                                 uint8_t operation,
                                 float *world,
                                 float *local);
} ct_editor_component_i0;

typedef struct ct_system_i0 {
    uint64_t name;
    ct_ecs_system_fce_t *process;

    uint64_t group;
    ce_ptr_pair_t0 before;
    ce_ptr_pair_t0 after;
} ct_system_i0;

typedef struct ct_system_group_i0 {
    uint64_t name;
    uint64_t group;
    ce_ptr_pair_t0 before;
    ce_ptr_pair_t0 after;
} ct_system_group_i0;

struct ct_ecs_a0 {
    ct_world_t0 (*create_world)(const char *name);

    void (*destroy_world)(ct_world_t0 world);

    void (*step)(ct_world_t0 world,
                 float dt);

    void (*buff_add_component)(ct_ecs_cmd_buffer_t *buffer,
                               ct_world_t0 world,
                               ct_entity_t0 ent,
                               const ct_component_pair_t0 *components,
                               uint32_t components_count);

    void (*buff_remove_component)(ct_ecs_cmd_buffer_t *buffer,
                                  ct_world_t0 world,
                                  ct_entity_t0 ent,
                                  const uint64_t *component_name,
                                  uint32_t name_count);
};

CE_MODULE(ct_ecs_a0);

struct ct_ecs_e_a0 {
    void (*create_entities)(ct_world_t0 world,
                            ct_entity_t0 *entity,
                            uint32_t count);

    void (*destroy_entities)(ct_world_t0 world,
                             ct_entity_t0 *entity,
                             uint32_t count);

    bool (*entity_alive)(ct_world_t0 world,
                         ct_entity_t0 entity);

    ct_entity_t0 (*spawn_entity)(ct_world_t0 world,
                                 uint64_t name);
};

CE_MODULE(ct_ecs_e_a0);

struct ct_ecs_c_a0 {
    //COMP
    bool (*has)(ct_world_t0 world,
                ct_entity_t0 ent,
                uint64_t *component_name,
                uint32_t name_count);

    ct_ecs_component_i0 *(*get_interface)(uint64_t name);

    ct_archemask_t0 (*combine_component)(const uint64_t *component_name,
                                         uint32_t n);

    void *(*get_all)(ct_world_t0 world,
                     uint64_t component_name,
                     ct_ecs_ent_chunk_o0 *chunk);

    void *(*get_one)(ct_world_t0 world,
                     uint64_t component_name,
                     ct_entity_t0 entity,
                     bool write);

    void (*add)(ct_world_t0 world,
                ct_entity_t0 ent,
                const ct_component_pair_t0 *components,
                uint32_t components_count);


    void (*remove)(ct_world_t0 world,
                   ct_entity_t0 ent,
                   const uint64_t *component_name,
                   uint32_t name_count);
};


CE_MODULE(ct_ecs_c_a0);

struct ct_ecs_q_a0 {
    void (*foreach)(ct_world_t0 world,
                    ct_ecs_query_t0 query,
                    uint32_t rq_version,
                    ct_ecs_foreach_fce_t fce,
                    void *data);

    void (*foreach_serial)(ct_world_t0 world,
                           ct_ecs_query_t0 query,
                           uint32_t rq_version,
                           ct_ecs_foreach_fce_t fce,
                           void *data);

    void (*collect_ents)(ct_world_t0 world,
                         ct_ecs_query_t0 query,
                         ct_entity_t0 **ents,
                         const ce_alloc_t0 *alloc);

    ct_entity_t0 (*first)(ct_world_t0 world,
                          ct_ecs_query_t0 query);
};

CE_MODULE(ct_ecs_q_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_ECS_H