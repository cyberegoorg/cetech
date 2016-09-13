#ifndef CETECH_BGFX_MATERIAL_H
#define CETECH_BGFX_MATERIAL_H

#include <celib/stringid/types.h>
#include <engine/entcom/types.h>

int material_resource_init();

void material_resource_shutdown();

typedef struct {
    union {
        handler_t h;
        u32 idx;
    };
} material_t;

material_t material_resource_create(stringid64_t name);

u32 material_get_texture_count(material_t material);

u32 material_find_slot(material_t material,
                       stringid64_t name);

void material_set_texture(material_t material,
                          u32 slot,
                          stringid64_t texture);

#endif //CETECH_BGFX_MATERIAL_H
