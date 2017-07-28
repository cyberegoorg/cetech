#ifndef CETECH_TEXTURE_BLOB_H
#define CETECH_TEXTURE_BLOB_H

#include <stdint.h>

namespace texture_blob {

    typedef struct {
        uint32_t blob_version;

        uint32_t size;
    } blob_t;

    inline const blob_t *get(void *data) {
        return (blob_t *) (data);
    }

//    inline uint32_t blob_size(const blob_t* blob) {
//        return (
//            sizeof(blob_t) +
//        0);
//    }

    inline uint32_t size(const blob_t *blob) {
        return blob->size;
    }

}

#endif // CETECH_TEXTURE_BLOB_H