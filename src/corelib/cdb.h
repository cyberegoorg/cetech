#ifndef CETECH_CDB_H
#define CETECH_CDB_H



//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <corelib/module.inl>

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

typedef void ct_cdb_obj_o;

typedef void (*ct_cdb_notify)(uint64_t obj,
                              const uint64_t *prop,
                              uint32_t prop_count,
                              void *data);

//==============================================================================
// Enums
//==============================================================================
enum ct_cdb_type {
    CDB_TYPE_NONE = 0,
    CDB_TYPE_UINT64,
    CDB_TYPE_PTR,
    CDB_TYPE_REF,
    CDB_TYPE_FLOAT,
    CDB_TYPE_BOOL,
    CDB_TYPE_STR,

    CDB_TYPE_VEC3,
    CDB_TYPE_VEC4,
    CDB_TYPE_MAT4,

    CDB_TYPE_SUBOBJECT,
    CDB_TYPE_BLOB,
};

//==============================================================================
// Interface
//==============================================================================

struct ct_cdb_a0 {
//    struct ct_cdb_t (*create_db)();

    struct ct_cdb_t (*db)();

    void (*register_notify)(uint64_t obj,
                            ct_cdb_notify notify,
                            void *data);

    uint64_t (*create_object)(struct ct_cdb_t db,
                              uint64_t type);

    uint64_t (*create_from)(struct ct_cdb_t db,
                            uint64_t obj);

    void (*destroy_db)(struct ct_cdb_t db);

    void (*destroy_object)(uint64_t obj);

    uint64_t (*type)(uint64_t obj);
    void (*set_type)(uint64_t obj, uint64_t type);

    void (*gc)();

    void (*dump)(uint64_t obj,
                 char **output,
                 struct ct_alloc *allocator);

    void (*load)(struct ct_cdb_t db,
                 const char *input,
                 uint64_t obj,
                 struct ct_alloc *allocator);

    // PROP
    bool (*prop_exist)(uint64_t object,
                       uint64_t key);

    enum ct_cdb_type (*prop_type)(uint64_t object,
                                  uint64_t key);

    void (*prop_keys)(uint64_t object,
                      uint64_t *keys);

    uint64_t (*prop_count)(uint64_t object);

    uint64_t (*parent)(uint64_t object);

    // WRITE
    ct_cdb_obj_o *(*write_begin)(uint64_t object);

    void (*write_commit)(ct_cdb_obj_o *writer);

    // SET
    void (*set_bool)(ct_cdb_obj_o *writer,
                     uint64_t property,
                     bool value);

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
                    uint64_t ref);

    void (*set_subobject)(ct_cdb_obj_o *writer,
                          uint64_t property,
                          uint64_t subobject);

    void (*set_blob)(ct_cdb_obj_o *writer,
                     uint64_t property,
                     void *blob,
                     uint64_t blob_size);


    void (*set_prefab)(uint64_t obj,
                       uint64_t prefab);


    // READ
    float (*read_float)(uint64_t object,
                        uint64_t property,
                        float defaultt);

    bool (*read_bool)(uint64_t object,
                        uint64_t property,
                        bool defaultt);


    void (*read_vec3)(uint64_t object,
                      uint64_t property,
                      float *value);

    void (*read_vec4)(uint64_t object,
                      uint64_t property,
                      float *value);

    void (*read_mat4)(uint64_t object,
                      uint64_t property,
                      float *value);

    const char *(*read_str)(uint64_t object,
                            uint64_t property,
                            const char *defaultt);

    uint64_t (*read_uint64)(uint64_t object,
                            uint64_t property,
                            uint64_t defaultt);

    void *(*read_ptr)(uint64_t object,
                      uint64_t property,
                      void *defaultt);

    uint64_t (*read_ref)(uint64_t object,
                         uint64_t property,
                         uint64_t defaultt);

    uint64_t (*read_subobject)(uint64_t object,
                               uint64_t property,
                               uint64_t defaultt);

    void *(*read_blob)(uint64_t object,
                       uint64_t property,
                       uint64_t *size,
                       void *defaultt);
};

CT_MODULE(ct_cdb_a0);

#endif //CETECH_CDB_H
