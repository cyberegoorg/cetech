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

#define CT_ECS_COMPONENT_I \
    CE_ID64_0("ct_ecs_component_i0", 0x9fda2263be703cb3ULL)

#define CT_ECS_EVENT_COMPONENT_SPAWN \
    CE_ID64_0("component_spawn", 0xa3f574429952b180ULL)

#define CT_ECS_EVENT_COMPONENT_DESTROY \
    CE_ID64_0("component_destroy", 0xca6890a5459ecb2cULL)

#define CT_ECS_EVENT_COMPONENT_CHANGE \
    CE_ID64_0("component_change", 0xb47624cc2d5cbbb7ULL)

#define CT_ECS_EVENT_ENT_LINK \
    CE_ID64_0("entity_link", 0xffcddf97336bbc9ULL)

#define CT_ECS_EVENT_ENT_UNLINK \
    CE_ID64_0("entity_unlink", 0x9caaf06ae5bc97e2ULL)

#define CT_ECS_WORLD_EVENT_CREATED \
    CE_ID64_0("ecs_world_evemt_created", 0xea233f148a521a8eULL)

#define CT_ECS_WORLD_EVENT_DESTROYED \
    CE_ID64_0("ecs_world_evemt_destroyed", 0x246df7bdf1f5ff81ULL)


typedef struct ct_resource_id_t0 ct_resource_id_t0;

typedef struct ct_entity_storage_o0 ct_entity_storage_o0;
typedef struct ct_ecs_ev_queue_o0 ct_ecs_ev_queue_o0;
typedef void ct_ecs_cmd_buffer_t;

typedef struct ct_world_t0 {
    uint64_t h;
} ct_world_t0;

typedef struct ct_entity_t0 {
    uint64_t h;
} ct_entity_t0;

typedef struct ct_ecs_component_i0 {
    const char *(*display_name)();

    uint64_t (*cdb_type)();

    uint64_t (*size)();

    void (*on_spawn)(uint64_t obj,
                     void *data);

    void (*on_change)(uint64_t obj,
                      void *data);

    void *(*get_interface)(uint64_t name_hash);
} ct_ecs_component_i0;

typedef struct ct_component_pair_t0 {
    uint64_t type;
    void *data;
} ct_component_pair_t0;

typedef void (*ct_process_fce_t)(ct_world_t0 world,
                                 ct_entity_t0 *ent,
                                 ct_entity_storage_o0 *item,
                                 uint32_t n,
                                 ct_ecs_cmd_buffer_t *cmd_buff,
                                 void *data);

typedef void (ct_simulate_fce_t)(ct_world_t0 world,
                                 float dt);


typedef struct ct_editor_component_i0 {
    void (*guizmo_get_transform)(uint64_t obj,
                                 float *world,
                                 float *local);

    void (*guizmo_set_transform)(uint64_t obj,
                                 uint8_t operation,
                                 float *world,
                                 float *local);
} ct_editor_component_i0;

struct ct_system_i0 {
    uint64_t (*name)();

    const uint64_t *(*before)(uint32_t *n);

    const uint64_t *(*after)(uint32_t *n);

    void (*on_create_world)(ct_world_t0 world,
                            ct_ecs_ev_queue_o0 *queue);

    ct_simulate_fce_t *simulation;
};

typedef struct ct_ecs_world_event_t0 {
    uint64_t type;
    ct_world_t0 world;
    union {
        struct {
            ct_entity_t0 ent;
            uint64_t type;
        } component;

        struct {
            ct_entity_t0 child;
            ct_entity_t0 parent;
        } link;
    };
} ct_ecs_world_event_t0;

struct ct_ecs_a0 {
    //WORLD
    ct_world_t0 (*create_world)();

    void (*destroy_world)(ct_world_t0 world);

    uint32_t (*world_num)();

    void (*all_world)(ct_world_t0 *worlds);


    ct_ecs_ev_queue_o0 *(*new_world_events_listener)(ct_world_t0 world);

    bool (*pop_world_events)(ct_ecs_ev_queue_o0 *q,
                             ct_ecs_world_event_t0 *ev);

    //ENT

    void (*create)(ct_world_t0 world,
                   ct_entity_t0 *entity,
                   uint32_t count);

    void (*destroy)(ct_world_t0 world,
                    ct_entity_t0 *entity,
                    uint32_t count);

    bool (*alive)(ct_world_t0 world,
                  ct_entity_t0 entity);

    ct_entity_t0 (*spawn)(ct_world_t0 world,
                          uint64_t name);


    bool (*has)(ct_world_t0 world,
                ct_entity_t0 ent,
                uint64_t *component_name,
                uint32_t name_count);

    void (*link)(ct_world_t0 world,
                 ct_entity_t0 parent,
                 ct_entity_t0 child);

    ct_entity_t0 (*parent)(ct_world_t0 world,
                           ct_entity_t0 entity);

    ct_entity_t0 (*first_child)(ct_world_t0 world,
                                ct_entity_t0 entity);

    ct_entity_t0 (*next_sibling)(ct_world_t0 world,
                                 ct_entity_t0 entity);

    //SYSTEM
    void (*step)(ct_world_t0 world,
                     float dt);

    void (*process)(ct_world_t0 world,
                    uint64_t components_mask,
                    ct_process_fce_t fce,
                    void *data);

    void (*process_serial)(ct_world_t0 world,
                           uint64_t components_mask,
                           ct_process_fce_t fce,
                           void *data);

    //COMP
    void (*component_changed)(ct_world_t0 world,
                              ct_entity_t0 ent,
                              uint64_t component);

    ct_ecs_component_i0 *(*get_interface)(uint64_t name);

    uint64_t (*mask)(uint64_t component_name);

    void *(*get_all)(uint64_t component_name,
                     ct_entity_storage_o0 *item);

    void *(*get_one)(ct_world_t0 world,
                     uint64_t component_name,
                     ct_entity_t0 entity);

    void (*add)(ct_world_t0 world,
                ct_entity_t0 ent,
                const ct_component_pair_t0 *components,
                uint32_t components_count);

    void (*add_buff)(ct_ecs_cmd_buffer_t *buffer,
                     ct_world_t0 world,
                     ct_entity_t0 ent,
                     const ct_component_pair_t0 *components,
                     uint32_t components_count);

    void (*remove)(ct_world_t0 world,
                   ct_entity_t0 ent,
                   const uint64_t *component_name,
                   uint32_t name_count);

    void (*remove_buff)(ct_ecs_cmd_buffer_t *buffer,
                        ct_world_t0 world,
                        ct_entity_t0 ent,
                        const uint64_t *component_name,
                        uint32_t name_count);

};

CE_MODULE(ct_ecs_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_ECS_H