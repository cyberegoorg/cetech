#ifndef CETECH_CDB_H
#define CETECH_CDB_H

#include <stddef.h>
#include <stdint.h>

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

struct ct_cdb_t {
    uint64_t idx;
};

struct ct_cdb_obj_t {
    void *_;
};

typedef void ct_cdb_obj_o;

typedef void (*ct_cdb_notify)(struct ct_cdb_obj_t *obj,
                              const uint64_t *prop,
                              uint32_t prop_count, void* data);


//==============================================================================
// Enums
//==============================================================================
enum ct_cdb_type {
    CDB_TYPE_NONE = 0,
    CDB_TYPE_UINT64,
    CDB_TYPE_PTR,
    CDB_TYPE_REF,
    CDB_TYPE_FLOAT,
    CDB_TYPE_STR,

    CDB_TYPE_VEC3,
    CDB_TYPE_VEC4,
    CDB_TYPE_MAT4,

    CDB_TYPE_SUBOBJECT,
};

//==============================================================================
// Interface
//==============================================================================

struct ct_cdb_a0 {
//    struct ct_cdb_t (*create_db)();

    struct ct_cdb_t (*global_db)();

    void (*register_notify)(struct ct_cdb_obj_t *obj,
                            ct_cdb_notify notify, void* data);

    struct ct_cdb_obj_t *(*create_object)(struct ct_cdb_t db,
                                          uint64_t type);

    struct ct_cdb_obj_t *(*create_from)(struct ct_cdb_t db,
                                        struct ct_cdb_obj_t *obj);

    void (*destroy_db)(struct ct_cdb_t db);

    void (*destroy_object)(struct ct_cdb_obj_t *obj);

    uint64_t (*type)(struct ct_cdb_obj_t *obj);

    void (*gc)();

    void (*dump)(struct ct_cdb_obj_t *obj,
                 char **output,
                 struct ct_alloc *allocator);

    void (*load)(struct ct_cdb_t db,
                 const char *input,
                 struct ct_cdb_obj_t * obj,
                 struct ct_alloc *allocator);

    // PROP
    bool (*prop_exist)(struct ct_cdb_obj_t *object,
                       uint64_t key);

    enum ct_cdb_type (*prop_type)(struct ct_cdb_obj_t *object,
                                  uint64_t key);

    void (*prop_keys)(struct ct_cdb_obj_t *object,
                      uint64_t *keys);

    uint64_t (*prop_count)(struct ct_cdb_obj_t *object);

    struct ct_cdb_obj_t *(*parent)(struct ct_cdb_obj_t *object);

    // WRITE
    ct_cdb_obj_o *(*write_begin)(struct ct_cdb_obj_t *object);

    void (*write_commit)(ct_cdb_obj_o *writer);

    // SET
    void (*set_float)(ct_cdb_obj_o *writer,
                      uint64_t property,
                      float value);

    void (*set_vec3)(ct_cdb_obj_o *writer,
                     uint64_t property,
                     const float *value);

    void (*set_vec4)(ct_cdb_obj_o *writer,
                     uint64_t property,
                     const float *value);

    void (*set_mat4)(ct_cdb_obj_o *writer,
                     uint64_t property,
                     const float *value);

    void (*set_str)(ct_cdb_obj_o *writer,
                    uint64_t property,
                    const char *value);

    void (*set_uint64)(ct_cdb_obj_o *writer,
                       uint64_t property,
                       uint64_t value);

    void (*set_ptr)(ct_cdb_obj_o *writer,
                    uint64_t property,
                    const void *value);

    void (*set_ref)(ct_cdb_obj_o *writer,
                    uint64_t property,
                    struct ct_cdb_obj_t *ref);

    void (*set_subobject)(ct_cdb_obj_o *writer,
                          uint64_t property,
                          struct ct_cdb_obj_t *subobject);

    void (*set_prefab)(struct ct_cdb_obj_t *obj,
                       struct ct_cdb_obj_t *prefab);


    // READ
    float (*read_float)(struct ct_cdb_obj_t *object,
                        uint64_t property,
                        float defaultt);

    void (*read_vec3)(struct ct_cdb_obj_t *object,
                      uint64_t property,
                      float *value);

    void (*read_vec4)(struct ct_cdb_obj_t *object,
                      uint64_t property,
                      float *value);

    void (*read_mat4)(struct ct_cdb_obj_t *object,
                      uint64_t property,
                      float *value);

    const char *(*read_str)(struct ct_cdb_obj_t *object,
                            uint64_t property,
                            const char *defaultt);

    uint64_t (*read_uint64)(struct ct_cdb_obj_t *object,
                            uint64_t property,
                            uint64_t defaultt);

    void *(*read_ptr)(struct ct_cdb_obj_t *object,
                      uint64_t property,
                      void *defaultt);

    struct ct_cdb_obj_t *(*read_ref)(struct ct_cdb_obj_t *object,
                                     uint64_t property,
                                     struct ct_cdb_obj_t *defaultt);

    struct ct_cdb_obj_t *(*read_subobject)(struct ct_cdb_obj_t *object,
                                           uint64_t property,
                                           struct ct_cdb_obj_t *defaultt);
};

#endif //CETECH_CDB_H
