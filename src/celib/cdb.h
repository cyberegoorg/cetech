#ifndef CE_CDB_H
#define CE_CDB_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <celib/module.inl>

#define CE_CDB_API \
    CE_ID64_0("ce_cdb_a0", 0xf069efc5d4120b7bULL)

#define CE_CDB_CHANGE \
    CE_ID64_0("change", 0x8694ed4881bfb631ULL)

#define CE_CDB_REMOVE \
    CE_ID64_0("remove", 0x36f36c7f8cda6d13ULL)

#define CDB_INSTANCE_PROP \
    CE_ID64_0("cdb_instance", 0xb0f74f1d9d7c645dULL)

struct ce_alloc;

struct ce_cdb_t {
    uint64_t idx;
};

struct ce_cdb_blob_t0 {
    void *data;
    uint64_t size;
};

union ce_cdb_value_u0 {
    uint64_t uint64;
    void *ptr;
    uint64_t ref;
    uint64_t subobj;

    float f;
    char *str;
    bool b;
    struct ce_cdb_blob_t0 blob;
};


struct ce_cdb_change_ev0 {
    uint64_t type;
    uint64_t obj;
    uint64_t prop;
    uint64_t prop_type;
    union ce_cdb_value_u0 new_value;
    union ce_cdb_value_u0 old_value;
};

typedef void ce_cdb_obj_o;


enum ce_cdb_type {
    CDB_TYPE_NONE = 0,
    CDB_TYPE_UINT64,
    CDB_TYPE_PTR,
    CDB_TYPE_REF,
    CDB_TYPE_FLOAT,
    CDB_TYPE_BOOL,
    CDB_TYPE_STR,
    CDB_TYPE_SUBOBJECT,
    CDB_TYPE_BLOB,
};

typedef bool (*ct_cdb_obj_loader)(uint64_t uid);

struct ce_cdb_a0 {
    void (*set_loader)(ct_cdb_obj_loader loader);

    struct ce_cdb_t (*db)();

    struct ce_cdb_t (*create_db)();

    void (*destroy_db)(struct ce_cdb_t db);


    uint64_t (*create_object)(struct ce_cdb_t db,
                              uint64_t type);

    void (*create_object_uid)(struct ce_cdb_t db,
                              uint64_t uid,
                              uint64_t type);

    uint64_t (*create_from)(struct ce_cdb_t db,
                            uint64_t obj);

    uint64_t (*clone)(struct ce_cdb_t db,
                      uint64_t obj);


    void (*set_from)(struct ce_cdb_t db,
                     uint64_t from,
                     uint64_t to);


    void (*destroy_object)(struct ce_cdb_t db,
                           uint64_t obj);

    uint64_t (*obj_type)(const ce_cdb_obj_o *reader);

    void (*set_type)(struct ce_cdb_t db,
                     uint64_t obj,
                     uint64_t type);

    uint64_t (*obj_key)(struct ce_cdb_t db,
                        uint64_t obj);

    //
    void (*move)(struct ce_cdb_t db,
                 uint64_t from_obj,
                 uint64_t to);

    //

    void (*gc)();

    //

    void (*dump_str)(struct ce_cdb_t db,
                     char **buffer,
                     uint64_t obj,
                     uint32_t level);

    void (*dump)(struct ce_cdb_t db,
                 uint64_t obj,
                 char **output,
                 struct ce_alloc *allocator);

    void (*load)(struct ce_cdb_t db,
                 const char *input,
                 uint64_t obj,
                 struct ce_alloc *allocator);

    uint64_t (*find_root)(struct ce_cdb_t _db,
                          uint64_t obj);

    // PROP
    bool (*prop_exist)(const ce_cdb_obj_o *reader,
                       uint64_t key);

    enum ce_cdb_type (*prop_type)(const ce_cdb_obj_o *reader,
                                  uint64_t key);

    const uint64_t *(*prop_keys)(const ce_cdb_obj_o *reader);

    uint64_t (*prop_count)(const ce_cdb_obj_o *reader);

    bool (*prop_equal)(const ce_cdb_obj_o *r1,
                       const ce_cdb_obj_o *r2,
                       uint64_t prorp);

    void (*prop_copy)(const ce_cdb_obj_o *from,
                      ce_cdb_obj_o *to,
                      uint64_t prorp);

    uint64_t (*parent)(const ce_cdb_obj_o *reader);

    void (*set_parent)(struct ce_cdb_t db,
                       uint64_t object,
                       uint64_t parent);

    // SET
    ce_cdb_obj_o *(*write_begin)(struct ce_cdb_t db,
                                 uint64_t object);

    void (*write_commit)(ce_cdb_obj_o *writer);


    void (*set_bool)(ce_cdb_obj_o *writer,
                     uint64_t property,
                     bool value);

    void (*set_float)(ce_cdb_obj_o *writer,
                      uint64_t property,
                      float value);

    void (*set_str)(ce_cdb_obj_o *writer,
                    uint64_t property,
                    const char *value);

    void (*set_uint64)(ce_cdb_obj_o *writer,
                       uint64_t property,
                       uint64_t value);

    void (*set_ptr)(ce_cdb_obj_o *writer,
                    uint64_t property,
                    const void *value);

    void (*set_blob)(ce_cdb_obj_o *writer,
                     uint64_t property,
                     void *blob,
                     uint64_t blob_size);

    void (*set_ref)(ce_cdb_obj_o *writer,
                    uint64_t property,
                    uint64_t ref);


    void (*set_subobject)(ce_cdb_obj_o *writer,
                          uint64_t property,
                          uint64_t subobject);

    void (*set_subobjectw)(ce_cdb_obj_o *writer,
                           uint64_t property,
                           ce_cdb_obj_o *sub_writer);


    void (*remove_property)(ce_cdb_obj_o *writer,
                            uint64_t property);


    // READ
    const ce_cdb_obj_o *(*read)(struct ce_cdb_t db,
                                uint64_t object);

    uint64_t (*read_instance_of)(const ce_cdb_obj_o *reader);

    const struct ce_cdb_change_ev0 *(*changed)(const ce_cdb_obj_o *reader,
                                               uint32_t *n);

    float (*read_float)(const ce_cdb_obj_o *reader,
                        uint64_t property,
                        float defaultt);

    bool (*read_bool)(const ce_cdb_obj_o *reader,
                      uint64_t property,
                      bool defaultt);

    const char *(*read_str)(const ce_cdb_obj_o *reader,
                            uint64_t property,
                            const char *defaultt);

    uint64_t (*read_uint64)(const ce_cdb_obj_o *reader,
                            uint64_t property,
                            uint64_t defaultt);

    void *(*read_ptr)(const ce_cdb_obj_o *reader,
                      uint64_t property,
                      void *defaultt);

    void *(*read_blob)(const ce_cdb_obj_o *reader,
                       uint64_t property,
                       uint64_t *size,
                       void *defaultt);

    uint64_t (*read_ref)(const ce_cdb_obj_o *reader,
                         uint64_t property,
                         uint64_t defaultt);

    uint64_t (*read_subobject)(const ce_cdb_obj_o *reader,
                               uint64_t property,
                               uint64_t defaultt);
};

CE_MODULE(ce_cdb_a0);

#endif //CE_CDB_H
