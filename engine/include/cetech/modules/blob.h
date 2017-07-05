#ifndef CETECH_BLOB_H
#define CETECH_BLOB_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

struct allocator;

//==============================================================================
// Typedefs
//==============================================================================

typedef void blob_instance_v0;

//==============================================================================
// Api
//==============================================================================

struct blob_v0 {
    blob_instance_v0 *inst;

    uint8_t *(*data)(blob_instance_v0 *inst);

    uint64_t (*size)(blob_instance_v0 *inst);

    void (*push)(blob_instance_v0 *inst,
                 void *data,
                 uint64_t size);
};

struct blob_api_v0 {
    struct blob_v0 *(*create)(struct allocator *allocator);

    void (*destroy)(struct blob_v0 *blob);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_BLOB_H
