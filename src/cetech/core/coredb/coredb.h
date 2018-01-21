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
struct ct_cdb_object_t {
    void *_;
};
struct ct_cdb_writer_t {
    void *_;
};

//==============================================================================
// Enums
//==============================================================================
enum ct_coredb_prop_type {
    COREDB_TYPE_NONE = 0,
    COREDB_TYPE_UINT32,
    COREDB_TYPE_PTR,
    COREDB_TYPE_REF,
    COREDB_TYPE_FLOAT,
    COREDB_TYPE_STRPTR,
};

//==============================================================================
// Interface
//==============================================================================

struct ct_coredb_a0 {
    struct ct_cdb_object_t *(*create_object)();

    bool (*prop_exist)(struct ct_cdb_object_t *object,
                       uint64_t key);

    enum ct_coredb_prop_type (*prop_type)(struct ct_cdb_object_t *object,
                                          uint64_t key);

    uint64_t * (*prop_keys)(struct ct_cdb_object_t *object);
    uint64_t (*prop_count)(struct ct_cdb_object_t *object);

    // WRITE
    struct ct_cdb_writer_t *
    (*write_begin)(struct ct_cdb_object_t *object);

    void (*write_commit)(struct ct_cdb_writer_t *writer);

    // SET
    void (*set_float)(struct ct_cdb_writer_t *object,
                      uint64_t property,
                      float value);

    void (*set_string)(struct ct_cdb_writer_t *object,
                       uint64_t property,
                       const char *value);

    void (*set_uint32)(struct ct_cdb_writer_t *object,
                       uint64_t property,
                       uint32_t value);

    void (*set_uint64)(struct ct_cdb_writer_t *object,
                       uint64_t property,
                       uint64_t value);

    void (*set_ptr)(struct ct_cdb_writer_t *object,
                    uint64_t property,
                    void *value);

    void (*set_ref)(struct ct_cdb_writer_t *object,
                    uint64_t property,
                    struct ct_cdb_object_t *ref);

    // READ
    float (*read_float)(struct ct_cdb_object_t *object,
                        uint64_t property,
                        float defaultt);

    const char *(*read_string)(struct ct_cdb_object_t *object,
                               uint64_t property,
                               const char *defaultt);

    uint32_t (*read_uint32)(struct ct_cdb_object_t *object,
                            uint64_t property,
                            uint32_t defaultt);

    uint64_t (*read_uint64)(struct ct_cdb_object_t *object,
                            uint64_t property,
                            uint64_t defaultt);

    void *(*read_ptr)(struct ct_cdb_object_t *object,
                      uint64_t property,
                      void *defaultt);

    struct ct_cdb_object_t *(*read_ref)(struct ct_cdb_object_t *object,
                                        uint64_t property,
                                        struct ct_cdb_object_t *defaultt);

};

#ifdef __cplusplus
}
#endif

#endif //CETECH_COREDB_H
