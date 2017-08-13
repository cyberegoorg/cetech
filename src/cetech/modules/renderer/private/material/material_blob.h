#ifndef CETECH_MATERIAL_BLOB_H
#define CETECH_MATERIAL_BLOB_H

#include <stdint.h>

namespace material_blob {

    typedef struct {
        uint32_t blob_version;

        uint32_t layer_count;
        uint32_t all_uniform_count;
        // uint64_t layer_names[layer_count];
        // uint64_t shader_name[layer_count];
        // uint32_t uniform_count[layer_count];
        // material_variable uniforms[all_uniform_count];
        // char[32] uniform_names[all_uniform_count];
        // uint32_t layer_offset[layer_count];
    } blob_t;

    inline const blob_t* get(void* data) {
        return (blob_t*)(data);
    }

    inline const uint32_t blob_size(const blob_t* blob) {
        return (
            sizeof(blob_t) +
            ( sizeof(uint64_t) * blob->layer_count ) +
            ( sizeof(uint64_t) * blob->layer_count ) +
            ( sizeof(uint32_t) * blob->layer_count ) +
            ( sizeof(material_variable) * blob->all_uniform_count ) +
            ( sizeof(char) * blob->all_uniform_count * 32) +
            ( sizeof(uint32_t) * blob->layer_count ) +
        0);
    }

    inline uint32_t layer_count(const blob_t* blob) {
        return blob->layer_count;
    }
    inline uint32_t all_uniform_count(const blob_t* blob) {
        return blob->all_uniform_count;
    }
    inline uint64_t* layer_names(const blob_t* blob) {
        return (uint64_t*)((blob) + 1);
    }
    inline uint64_t* shader_name(const blob_t* blob) {
        return ((uint64_t*) (layer_names(blob) + (blob->layer_count)));
    }
    inline uint32_t* uniform_count(const blob_t* blob) {
        return ((uint32_t*) (shader_name(blob) + (blob->layer_count)));
    }
    inline material_variable* uniforms(const blob_t* blob) {
        return ((material_variable*) (uniform_count(blob) + (blob->layer_count)));
    }
    inline char* uniform_names(const blob_t* blob) {
        return ((char*) (uniforms(blob) + (blob->all_uniform_count)));
    }
    inline uint32_t* layer_offset(const blob_t* blob) {
        return ((uint32_t*) (uniform_names(blob) + (blob->all_uniform_count*32)));
    }

}

#endif // CETECH_MATERIAL_BLOB_H