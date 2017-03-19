#ifndef CETECH_MATERIAL_BLOB_H
#define CETECH_MATERIAL_BLOB_H


#include <celib/string/stringid.h>

typedef struct material_blob {
    stringid64_t shader_name;
    u32 uniforms_count;
    u32 texture_count;
    u32 cel_vec4f_count;
    u32 mat33f_count;
    u32 cel_mat44f_count;
    // material_blob + 1                 | char[32]     uniform_names [uniform_count]
    // uniform_names     + uniform_count | stringid64_t texture_names [texture_count]
    // texture_names     + texture_count | vec4f        cel_vec4f_value [cel_vec4f_count]
    // cel_vec4f_value       + cel_vec4f_count   | mat44f       mat33f_value [mat33f_count]
    // mat33f_count      + mat33f_count  | mat33f       cel_mat44f_value [cel_mat44f_count]
} material_blob_t;

#define material_blob_uniform_names(r)   ((char*)                  ((r)+1))
#define material_blob_uniform_texture(r) ((stringid64_t*)          ((material_blob_uniform_names(r)+(32*((r)->uniforms_count)))))
#define material_blob_uniform_vec4f(r)   ((cel_vec4f_t*)               ((material_blob_uniform_texture(r)+((r)->texture_count))))
#define material_blob_uniform_mat33f(r)  ((mat33f_t*)              ((material_blob_uniform_vec4f(r)+((r)->cel_vec4f_count))))
#define material_blob_uniform_mat44f(r)  ((cel_mat44f_t*)              ((material_blob_uniform_mat33f(r)+((r)->mat33f_count))))
#define material_blob_uniform_bgfx(r)    ((bgfx_uniform_handle_t*) ((material_blob_uniform_vec4f(r)+((r)->cel_vec4f_count))))


#endif //CETECH_MATERIAL_BLOB_H
