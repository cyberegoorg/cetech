#ifndef CETECH_COREDB_H
#define CETECH_COREDB_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdbool.h>

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Typedefs
//==============================================================================
struct ct_coredb_object_t {
    void *_;
};
struct ct_coredb_writer_t {
    void *_;
};

//==============================================================================
// Enums
//==============================================================================
enum ct_coredb_prop_type {
    COREDB_TYPE_NONE = 0,
    COREDB_TYPE_UINT32,
    COREDB_TYPE_FLOAT,
    COREDB_TYPE_STRPTR,
};

//==============================================================================
// Interface
//==============================================================================

struct ct_coredb_a0 {
    ct_coredb_object_t *(*create_object)();

    bool (*prop_exist)(ct_coredb_object_t *object, uint64_t key);
    ct_coredb_prop_type (*prop_type)(ct_coredb_object_t *object, uint64_t key);

    // WRITE
    ct_coredb_writer_t *(*write_begin)(ct_coredb_object_t *object);
    void (*write_commit)(ct_coredb_writer_t *writer);

    // SET
    void (*set_float)(ct_coredb_writer_t *object,
                      uint64_t property,
                      float value);

    void (*set_string)(ct_coredb_writer_t *object,
                       uint64_t property,
                       const char *value);

    void (*set_uint32)(ct_coredb_writer_t *object,
                       uint64_t property,
                       uint32_t value);

    // READ
    float (*read_float)(ct_coredb_object_t *object,
                        uint64_t property,
                        float defaultt);

    const char *(*read_string)(ct_coredb_object_t *object,
                               uint64_t property,
                               const char *defaultt);

    uint32_t (*read_uint32)(ct_coredb_object_t *object,
                        uint64_t property,
                        uint32_t defaultt);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_COREDB_H
