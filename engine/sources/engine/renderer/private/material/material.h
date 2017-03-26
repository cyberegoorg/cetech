#ifndef CETECH_MATERIAL_H
#define CETECH_MATERIAL_H

material_t material_create(stringid64_t name);

u32 material_get_texture_count(material_t material);

void material_set_texture(material_t material,
                          const char *slot,
                          stringid64_t texture);

void material_set_vec4f(material_t material,
                        const char *slot,
                        cel_vec4f_t v);

void material_set_mat33f(material_t material,
                         const char *slot,
                         mat33f_t v);

void material_set_mat44f(material_t material,
                         const char *slot,
                         cel_mat44f_t v);


void material_use(material_t material);
void material_submit(material_t material);

#endif //CETECH_MATERIAL_H
