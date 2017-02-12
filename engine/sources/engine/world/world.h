#ifndef CETECH_WORLD_SYSTEM_H
#define CETECH_WORLD_SYSTEM_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/handler/handlerid.h"

typedef struct {
    handler_t h;
} world_t;

typedef void (*world_on_created_t)(world_t world);

typedef void (*world_on_destroy_t)(world_t world);

typedef void (*world_on_update_t)(world_t world,
                                  float dt);

typedef struct {
    world_on_created_t on_created;
    world_on_destroy_t on_destroy;
    world_on_update_t on_update;
} world_callbacks_t;

//==============================================================================
// Interface
//==============================================================================

struct WorldApiV1 {
    void (*register_callback)(world_callbacks_t clb);
    world_t (*create)();
    void (*destroy)(world_t world);
    void (*update)(world_t world, float dt);
};


#endif //CETECH_WORLD_SYSTEM_H
