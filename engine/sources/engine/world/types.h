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


#endif //CETECH_WORLD_TYPES_H
