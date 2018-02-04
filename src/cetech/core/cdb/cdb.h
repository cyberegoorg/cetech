#ifndef CETECH_CDB_H
#define CETECH_CDB_H

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
struct ct_alloc;

struct ct_cdb_obj_t {
    void *_;
};

struct ct_cdb_writer_t {
    void *_;
};

//==============================================================================
// Enums
//==============================================================================
enum ct_cdb_prop_type {
    CDB_TYPE_NONE = 0,
    CDB_TYPE_UINT32,
    CDB_TYPE_UINT64,
    CDB_TYPE_PTR,
    CDB_TYPE_REF,
    CDB_TYPE_FLOAT,
    CDB_TYPE_STRPTR,
    CDB_TYPE_VEC3,
    CDB_TYPE_VEC4,
};

//==============================================================================
// Interface
//==============================================================================

typedef void (*ct_cdb_chg_notify)(struct ct_cdb_obj_t* obj, uint64_t *prop, uint32_t prop_count);

struct ct_cdb_a0 {
    struct ct_cdb_obj_t *(*create_object)();

    void (*dump)(struct ct_cdb_obj_t *obj, char** output, struct ct_alloc* allocator);

    void (*load)(struct ct_cdb_obj_t *_obj, const char* input, struct ct_alloc* allocator);

    void (*register_notify)(struct ct_cdb_obj_t* obj, ct_cdb_chg_notify clb);

    bool (*prop_exist)(struct ct_cdb_obj_t *object,
                       uint64_t key);

    enum ct_cdb_prop_type (*prop_type)(struct ct_cdb_obj_t *object,
                                       uint64_t key);

    uint64_t *(*prop_keys)(struct ct_cdb_obj_t *object);

    uint64_t (*prop_count)(struct ct_cdb_obj_t *object);


    // WRITE
    struct ct_cdb_writer_t *
    (*write_begin)(struct ct_cdb_obj_t *object);

    void (*write_commit)(struct ct_cdb_writer_t *writer);

    // SET
    void (*set_float)(struct ct_cdb_writer_t *object,
                      uint64_t property,
                      float value);

    void (*set_vec3)(struct ct_cdb_writer_t *object,
                     uint64_t property,
                     const float *value);

    void (*set_vec4)(struct ct_cdb_writer_t *object,
                     uint64_t property,
                     const float *value);


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
                    struct ct_cdb_obj_t *ref);

    // READ
    float (*read_float)(struct ct_cdb_obj_t *object,
                        uint64_t property,
                        float defaultt);

    void (*read_vec3)(struct ct_cdb_obj_t *object,
                              uint64_t property,
                              float* value);

    void (*read_vec4)(struct ct_cdb_obj_t *object,
                      uint64_t property,
                      float* value);

    const char *(*read_str)(struct ct_cdb_obj_t *object,
                            uint64_t property,
                            const char *defaultt);

    uint32_t (*read_uint32)(struct ct_cdb_obj_t *object,
                            uint64_t property,
                            uint32_t defaultt);

    uint64_t (*read_uint64)(struct ct_cdb_obj_t *object,
                            uint64_t property,
                            uint64_t defaultt);

    void *(*read_ptr)(struct ct_cdb_obj_t *object,
                      uint64_t property,
                      void *defaultt);

    struct ct_cdb_obj_t *(*read_ref)(struct ct_cdb_obj_t *object,
                                     uint64_t property,
                                     struct ct_cdb_obj_t *defaultt);

};

#ifdef __cplusplus
}
#endif

#endif //CETECH_CDB_H
