#ifndef CETECH_MATERIAL_BLOB_H
#define CETECH_MATERIAL_BLOB_H

#include <stdint.h>
#include <cetech/celib/math_types.h>

namespace material_blob {

    typedef struct {
        uint32_t blob_version;

        uint64_t shader_name;
        uint32_t uniforms_count;
        uint32_t texture_count;
        uint32_t vec4f_count;
        uint32_t mat33f_count;
        uint32_t mat44f_count;
        // char[32] uniform_names[uniforms_count];
        // uint64_t texture_names[texture_count];
        // vec4f_t vec4f_value[vec4f_count];
        // mat44f_t mat44f_value[mat44f_count];
        // mat33f_t mat33f_value[mat33f_count];
    } blob_t;

    inline const blob_t* get(void* data) {
        return (blob_t*)(data);
    }

    inline const uint32_t size(const blob_t* blob) {
        return (
            sizeof(blob_t) +
            ( sizeof(char) * blob->uniforms_count * 32) +
            ( sizeof(uint64_t) * blob->texture_count ) +
            ( sizeof(vec4f_t) * blob->vec4f_count ) +
            ( sizeof(mat44f_t) * blob->mat44f_count ) +
            ( sizeof(mat33f_t) * blob->mat33f_count ) +
        0);
    }

    inline uint64_t shader_name(const blob_t* blob) {
        return blob->shader_name;
    }
    inline uint32_t uniforms_count(const blob_t* blob) {
        return blob->uniforms_count;
    }
    inline uint32_t texture_count(const blob_t* blob) {
        return blob->texture_count;
    }
    inline uint32_t vec4f_count(const blob_t* blob) {
        return blob->vec4f_count;
    }
    inline uint32_t mat33f_count(const blob_t* blob) {
        return blob->mat33f_count;
    }
    inline uint32_t mat44f_count(const blob_t* blob) {
        return blob->mat44f_count;
    }
    inline char* uniform_names(const blob_t* blob) {
        return (char*)((blob) + 1);
    }
    inline uint64_t* texture_names(const blob_t* blob) {
        return ((uint64_t*) (uniform_names(blob) + (blob->uniforms_count*32)));
    }
    inline vec4f_t* vec4f_value(const blob_t* blob) {
        return ((vec4f_t*) (texture_names(blob) + (blob->texture_count)));
    }
    inline mat44f_t* mat44f_value(const blob_t* blob) {
        return ((mat44f_t*) (vec4f_value(blob) + (blob->vec4f_count)));
    }
    inline mat33f_t* mat33f_value(const blob_t* blob) {
        return ((mat33f_t*) (mat44f_value(blob) + (blob->mat44f_count)));
    }

}

#endif // CETECH_MATERIAL_BLOB_H