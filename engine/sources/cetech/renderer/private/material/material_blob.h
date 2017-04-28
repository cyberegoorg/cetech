#ifndef CETECH_MATERIAL_BLOB_H
#define CETECH_MATERIAL_BLOB_H

#include <celib/stringid.h>
#include <celib/math_types.h>

typedef struct material_blob {
    stringid64_t shader_name;
    u32 uniforms_count;
    u32 texture_count;
    u32 vec4f_count;
    u32 mat33f_count;
    u32 mat44f_count;
    // char[32] uniform_names[uniforms_count];
    // stringid64_t texture_names[texture_count];
    // cel_vec4f_t vec4f_value[vec4f_count];
    // cel_mat44f_t mat44f_value[mat44f_count];
    // mat33f_t mat33f_value[mat33f_count];
} material_blob_t;

#define material_blob_uniform_names(r) ((char*) ((r) + 1))
#define material_blob_texture_names(r) ((stringid64_t*) (material_blob_uniform_names(r) + ((r)->uniforms_count*32)))
#define material_blob_vec4f_value(r) ((cel_vec4f_t*) (material_blob_texture_names(r) + ((r)->texture_count)))
#define material_blob_mat44f_value(r) ((cel_mat44f_t*) (material_blob_vec4f_value(r) + ((r)->vec4f_count)))
#define material_blob_mat33f_value(r) ((mat33f_t*) (material_blob_mat44f_value(r) + ((r)->mat44f_count)))

#endif // CETECH_MATERIAL_BLOB_H