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

//u32 material_find_slot(material_t material,
//                       const char *name);

void material_set_texture(material_t material,
                          const char *slot,
                          stringid64_t texture);

void material_set_vec4f(material_t material,
                        const char *slot,
                        vec4f_t v);

void material_use(material_t material);

void material_submit(material_t material);

#endif //CETECH_BGFX_MATERIAL_H
