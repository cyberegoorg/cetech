#ifndef CETECH_CORE_TYPES_H
#define CETECH_CORE_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/containers/map.h"
#include "celib/os/vio_types.h"
#include "celib/handler/handlerid.h"
#include "celib/yaml/yaml.h"
#include "celib/types.h"

#include "engine/components/types.h"

//==============================================================================
// World
//==============================================================================


typedef struct {
    handler_t h;
} world_t;

//==============================================================================
// Level
//==============================================================================

typedef struct {
    u32 idx;
} level_t;


//==============================================================================
// Typedef
//==============================================================================

typedef struct {
    union {
        handler_t h;
        u32 idx;
    };
} entity_t;

ARRAY_PROTOTYPE(entity_t)

MAP_PROTOTYPE(entity_t);

typedef int (*component_compiler_t)(yaml_node_t body,
                                    ARRAY_T(u8) *data);

typedef void (*component_on_world_create_t)(world_t world);

typedef void (*component_on_world_destroy_t)(world_t world);

typedef void (*component_destroyer_t)(world_t world,
                                      entity_t *ents,
                                      size_t ent_count);

typedef void (*component_spawner_t)(world_t world,
                                    entity_t *ents,
                                    u32 *cent,
                                    u32 *ents_parent,
                                    size_t ent_count,
                                    void *data);


//==============================================================================
// Task
//==============================================================================


typedef struct {
    u32 id;
} task_t;

static const task_t task_null = (task_t) {.id = 0};

typedef void (*task_work_t)(void *data);

enum workers {
    TASK_WORKER_MAIN = 0,
    TASK_WORKER1 = 1,
    TASK_WORKER2 = 2,
    TASK_WORKER3 = 3,
    TASK_WORKER4 = 4,
    TASK_WORKER5 = 5,
    TASK_WORKER6 = 6,
    TASK_WORKER7 = 7,
    TASK_MAX_WORKERS,
};

enum task_affinity {
    TASK_AFFINITY_NONE = 0,
    TASK_AFFINITY_MAIN = 1,
    TASK_AFFINITY_WORKER1 = 2,
    TASK_AFFINITY_WORKER2 = 3,
    TASK_AFFINITY_WORKER3 = 4,
    TASK_AFFINITY_WORKER4 = 5,
    TASK_AFFINITY_WORKER5 = 6,
    TASK_AFFINITY_WORKER6 = 7,
    TASK_AFFINITY_WORKER7 = 8,
    TASK_AFFINITY_MAX
};

CE_STATIC_ASSERT(TASK_AFFINITY_MAX == TASK_MAX_WORKERS + 1);


enum task_priority {
    TASK_PRIORITY_HIGH = 0,
    TASK_PRIORITY_NORMAL = 1,
    TASK_PRIORITY_LOW = 2,
    TASK_PRIORITY_COUNT,
};


#endif //CETECH_CORE_TYPES_H
