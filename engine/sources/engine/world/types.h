#ifndef CETECH_WORLD_TYPES_H
#define CETECH_WORLD_TYPES_H

#include <celib/handler/handlerid.h>

typedef struct {
    handler_t h;
} world_t;


typedef struct {
    u32 idx;
} level_t;

typedef struct {
    u32 idx;
} transform_t;

typedef void (*world_on_created_t)(world_t world);

typedef void (*world_on_destroy_t)(world_t world);

typedef void (*world_on_update_t)(world_t world, float dt);


#endif //CETECH_WORLD_TYPES_H
