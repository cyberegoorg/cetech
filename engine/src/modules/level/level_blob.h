#ifndef CETECH_LEVEL_BLOB_H
#define CETECH_LEVEL_BLOB_H

#include <stdint.h>

namespace level_blob {

    typedef struct {
        uint32_t blob_version;

        uint32_t entities_count;
        // uint64_t names[entities_count];
        // uint32_t offset[entities_count];
        // uint8_t data[*];
    } blob_t;

    inline const blob_t* get(void* data) {
        return (blob_t*)(data);
    }

    inline const uint32_t blob_size(const blob_t* blob) {
        return (
            sizeof(blob_t) +
            ( sizeof(uint64_t) * blob->entities_count ) +
            ( sizeof(uint32_t) * blob->entities_count ) +
        0);
    }

    inline uint32_t entities_count(const blob_t* blob) {
        return blob->entities_count;
    }
    inline uint64_t* names(const blob_t* blob) {
        return (uint64_t*)((blob) + 1);
    }
    inline uint32_t* offset(const blob_t* blob) {
        return ((uint32_t*) (names(blob) + (blob->entities_count)));
    }
    inline uint8_t* data(const blob_t* blob) {
        return ((uint8_t*) (offset(blob) + (blob->entities_count)));
    }

}

#endif // CETECH_LEVEL_BLOB_H