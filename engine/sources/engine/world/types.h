#ifndef CETECH_WORLD_TYPES_H
#define CETECH_WORLD_TYPES_H

#include <celib/handler/handlerid.h>
#include <celib/math/types.h>
#include <celib/string/stringid.h>
#include <celib/yaml/yaml.h>

typedef struct {
    handler_t h;
} world_t;

typedef struct {
    u32 idx;
} level_t;

typedef struct {
    u32 idx;
} transform_t;

typedef struct {
    u32 idx;
} camera_t;

typedef struct {
    u32 idx;
} scene_node_t;

typedef void (*world_on_created_t)(world_t world);

typedef void (*world_on_destroy_t)(world_t world);

typedef void (*world_on_update_t)(world_t world,
                                  float dt);

typedef struct {
    world_on_created_t on_created;
    world_on_destroy_t on_destroy;
    world_on_update_t on_update;
} world_callbacks_t;


#endif //CETECH_WORLD_TYPES_H
