#ifndef CETECH_BLOB_H
#define CETECH_BLOB_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

struct ct_allocator;

//==============================================================================
// Typedefs
//==============================================================================

typedef void ct_blob_instance_v0;

//==============================================================================
// Api
//==============================================================================

struct ct_blob_v0 {
    ct_blob_instance_v0 *inst;

    uint8_t *(*data)(ct_blob_instance_v0 *inst);

    uint64_t (*size)(ct_blob_instance_v0 *inst);

    void (*push)(ct_blob_instance_v0 *inst,
                 void *data,
                 uint64_t size);
};

struct ct_blob_api_v0 {
    struct ct_blob_v0 *(*create)(struct ct_allocator *allocator);

    void (*destroy)(struct ct_blob_v0 *blob);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_BLOB_H
