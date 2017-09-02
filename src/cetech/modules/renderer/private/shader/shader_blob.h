#ifndef CETECH_SHADER_BLOB_H
#define CETECH_SHADER_BLOB_H

#include <stdint.h>

namespace shader_blob {

    typedef struct {
        uint32_t blob_version;

        uint32_t vs_size;
        uint32_t fs_size;
        // uint8_t vs[vs_size];
        // uint8_t fs[fs_size];
    } blob_t;

    inline const blob_t *get(void *data) {
        return (blob_t *) (data);
    }

    inline uint32_t blob_size(const blob_t *blob) {
        return (
                sizeof(blob_t) +
                (sizeof(uint8_t) * blob->vs_size) +
                (sizeof(uint8_t) * blob->fs_size) +
                0);
    }

    inline uint32_t vs_size(const blob_t *blob) {
        return blob->vs_size;
    }

    inline uint32_t fs_size(const blob_t *blob) {
        return blob->fs_size;
    }

    inline uint8_t *vs(const blob_t *blob) {
        return (uint8_t *) ((blob) + 1);
    }

    inline uint8_t *fs(const blob_t *blob) {
        return ((uint8_t *) (vs(blob) + (blob->vs_size)));
    }

}

#endif // CETECH_SHADER_BLOB_H