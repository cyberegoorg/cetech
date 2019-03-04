#ifndef CE_CDB_H
#define CE_CDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib_types.h"

#define CE_CDB_API \
    CE_ID64_0("ce_cdb_a0", 0xf069efc5d4120b7bULL)

#define CE_CDB_CHANGE \
    CE_ID64_0("change", 0x8694ed4881bfb631ULL)

#define CE_CDB_REMOVE \
    CE_ID64_0("remove", 0x36f36c7f8cda6d13ULL)

#define CDB_INSTANCE_PROP \
    CE_ID64_0("cdb_instance", 0xb0f74f1d9d7c645dULL)

#define CE_CDB_MOVE \
    CE_ID64_0("move", 0x33603ac62788b5c5ULL)

#define CE_CDB_OBJSET_ADD \
    CE_ID64_0("objset_add", 0x7390985657b354e9ULL)

#define CE_CDB_OBJSET_REMOVE \
    CE_ID64_0("objset_remove", 0xd973a462dce1b7c8ULL)

typedef struct ce_alloc_t0 ce_alloc_t0;

typedef enum ce_cdb_type_e0 {
    CDB_TYPE_NONE = 0,
    CDB_TYPE_UINT64 = 1,
    CDB_TYPE_PTR = 2,
    CDB_TYPE_REF = 3,
    CDB_TYPE_FLOAT = 4,
    CDB_TYPE_BOOL = 5,
    CDB_TYPE_STR = 6,
    CDB_TYPE_SUBOBJECT = 7,
    CDB_TYPE_BLOB = 8,
    CDB_TYPE_SET_SUBOBJECT = 9,
} ce_cdb_type_e0;

typedef enum ce_cdb_prop_flag_e0 {
    CDB_PROP_FLAG_NONE = 0,
    CDB_PROP_FLAG_UNPACK = 1 << 0,
} ce_cdb_flag_e0;


typedef struct ct_cdb_ev_queue_o0 ct_cdb_ev_queue_o0;
typedef struct ce_cdb_obj_o0 ce_cdb_obj_o0;

typedef struct ce_cdb_t0 {
    uint64_t idx;
} ce_cdb_t0;

typedef struct ce_cdb_blob_t0 {
    void *data;
    uint64_t size;
} ce_cdb_blob_t0;

typedef union ce_cdb_value_u0 {
    uint64_t uint64;
    void *ptr;
    uint64_t ref;
    uint64_t subobj;

    float f;
    char *str;
    bool b;
    uint32_t blob;
    uint32_t set;
} ce_cdb_value_u0;

typedef struct ce_cdb_change_ev_t0 {
    uint64_t type;
    uint64_t obj;
    uint64_t prop;
    uint64_t prop_type;
    union {
        struct {
            union ce_cdb_value_u0 new_value;
            union ce_cdb_value_u0 old_value;
        };
        struct {
            uint64_t to;
            union ce_cdb_value_u0 value;
        };
    };
} ce_cdb_change_ev_t0;

typedef bool (*ct_cdb_obj_loader_t0)(uint64_t uid);

typedef struct ce_cdb_prop_def_t0 {
    const char *name;
    uint8_t type;
    ce_cdb_value_u0 value;
    uint64_t obj_type;
    uint64_t flags;
} ce_cdb_prop_def_t0;

typedef struct ce_cdb_type_def_t0 {
    ce_cdb_prop_def_t0 *defs;
    uint32_t num;
} ce_cdb_type_def_t0;

struct ce_cdb_a0 {
    void (*set_loader)(ct_cdb_obj_loader_t0 loader);

    ce_cdb_t0 (*db)();

    ce_cdb_t0 (*create_db)(uint64_t max_objects);

    void (*destroy_db)(ce_cdb_t0 db);


    void (*reg_obj_type)(uint64_t type,
                         const ce_cdb_prop_def_t0 *prop_def,
                         uint32_t n);

    const ce_cdb_type_def_t0 *(*obj_type_def)(uint64_t type);

    uint64_t (*gen_uid)(ce_cdb_t0 db);

    uint64_t (*create_object)(ce_cdb_t0 db,
                              uint64_t type);

    void (*create_object_uid)(ce_cdb_t0 db,
                              uint64_t uid,
                              uint64_t type);

    uint64_t (*create_from)(ce_cdb_t0 db,
                            uint64_t obj);

    void (*set_instance_of)(ce_cdb_t0 db,
                            uint64_t from,
                            uint64_t to);


    void (*destroy_object)(ce_cdb_t0 db,
                           uint64_t obj);

    uint64_t (*obj_type)(ce_cdb_t0 db,
                         uint64_t obj);

    void (*set_type)(ce_cdb_t0 db,
                     uint64_t obj,
                     uint64_t type);

    //
    void (*move_obj)(ce_cdb_t0 db,
                     uint64_t from_obj,
                     uint64_t to);

    void (*move_prop)(ce_cdb_obj_o0 *from_w,
                      ce_cdb_obj_o0 *to_w,
                      uint64_t prop);

    void (*move_objset_obj)(ce_cdb_obj_o0 *from_w,
                            ce_cdb_obj_o0 *to_w,
                            uint64_t prop,
                            uint64_t obj);

    //

    void (*gc)();

    //

    void (*dump_str)(ce_cdb_t0 db,
                     char **buffer,
                     uint64_t obj,
                     uint32_t level);

    void (*log_obj)(const char *where,
                    ce_cdb_t0 db,
                    uint64_t obj);

    void (*dump)(ce_cdb_t0 db,
                 uint64_t obj,
                 char **output,
                 ce_alloc_t0 *allocator);

    void (*load)(ce_cdb_t0 db,
                 const char *input,
                 uint64_t obj,
                 ce_alloc_t0 *allocator);

    uint64_t (*find_root)(ce_cdb_t0 _db,
                          uint64_t obj);

    // PROP
    bool (*prop_exist)(const ce_cdb_obj_o0 *reader,
                       uint64_t key);

    enum ce_cdb_type_e0 (*prop_type)(const ce_cdb_obj_o0 *reader,
                                     uint64_t key);

    const uint64_t *(*prop_keys)(const ce_cdb_obj_o0 *reader);

    uint64_t (*prop_count)(const ce_cdb_obj_o0 *reader);

    bool (*prop_equal)(const ce_cdb_obj_o0 *r1,
                       const ce_cdb_obj_o0 *r2,
                       uint64_t prorp);

    void (*prop_copy)(const ce_cdb_obj_o0 *from,
                      ce_cdb_obj_o0 *to,
                      uint64_t prorp);

    uint64_t (*parent)(ce_cdb_t0 db,
                       uint64_t object);

    // SET
    ce_cdb_obj_o0 *(*write_begin)(ce_cdb_t0 db,
                                  uint64_t object);

    void (*write_commit)(ce_cdb_obj_o0 *writer);

    bool (*write_try_commit)(ce_cdb_obj_o0 *writer);


    void (*set_bool)(ce_cdb_obj_o0 *writer,
                     uint64_t property,
                     bool value);

    void (*set_float)(ce_cdb_obj_o0 *writer,
                      uint64_t property,
                      float value);

    void (*set_str)(ce_cdb_obj_o0 *writer,
                    uint64_t property,
                    const char *value);

    void (*set_uint64)(ce_cdb_obj_o0 *writer,
                       uint64_t property,
                       uint64_t value);

    void (*set_ptr)(ce_cdb_obj_o0 *writer,
                    uint64_t property,
                    const void *value);

    void (*set_blob)(ce_cdb_obj_o0 *writer,
                     uint64_t property,
                     void *blob,
                     uint64_t blob_size);

    void (*set_ref)(ce_cdb_obj_o0 *writer,
                    uint64_t property,
                    uint64_t ref);


    void (*set_subobject)(ce_cdb_obj_o0 *writer,
                          uint64_t property,
                          uint64_t subobject);

    void (*set_subobjectw)(ce_cdb_obj_o0 *writer,
                           uint64_t property,
                           ce_cdb_obj_o0 *sub_writer);

    void (*objset_add_obj)(ce_cdb_obj_o0 *writer,
                           uint64_t property,
                           uint64_t obj);

    void (*objset_remove_obj)(ce_cdb_obj_o0 *writer,
                              uint64_t property,
                              uint64_t obj);

    void (*remove_property)(ce_cdb_obj_o0 *writer,
                            uint64_t property);

    const uint64_t *(*destroyed)(ce_cdb_t0 db,
                                 uint32_t *n);

    ct_cdb_ev_queue_o0 *(*new_changed_obj_listener)(ce_cdb_t0 db);

    bool (*pop_changed_obj)(ct_cdb_ev_queue_o0 *q,
                            uint64_t *obj);

    ct_cdb_ev_queue_o0 *(*new_obj_listener)(ce_cdb_t0 db);

    bool (*pop_obj_events)(ct_cdb_ev_queue_o0 *q,
                           ce_cdb_change_ev_t0 *ev);

    // READ
    const ce_cdb_obj_o0 *(*read)(ce_cdb_t0 db,
                                 uint64_t object);

    uint64_t (*read_to)(ce_cdb_t0 db,
                        uint64_t object,
                        void *to,
                        size_t max_size);

    uint64_t (*read_prop_to)(ce_cdb_t0 db,
                             uint64_t object,
                             uint64_t prop,
                             void *to,
                             size_t max_size);

    uint64_t (*read_instance_of)(const ce_cdb_obj_o0 *reader);


    float (*read_float)(const ce_cdb_obj_o0 *reader,
                        uint64_t property,
                        float defaultt);

    bool (*read_bool)(const ce_cdb_obj_o0 *reader,
                      uint64_t property,
                      bool defaultt);

    const char *(*read_str)(const ce_cdb_obj_o0 *reader,
                            uint64_t property,
                            const char *defaultt);

    uint64_t (*read_uint64)(const ce_cdb_obj_o0 *reader,
                            uint64_t property,
                            uint64_t defaultt);

    void *(*read_ptr)(const ce_cdb_obj_o0 *reader,
                      uint64_t property,
                      void *defaultt);

    void *(*read_blob)(const ce_cdb_obj_o0 *reader,
                       uint64_t property,
                       uint64_t *size,
                       void *defaultt);

    uint64_t (*read_ref)(const ce_cdb_obj_o0 *reader,
                         uint64_t property,
                         uint64_t defaultt);

    uint64_t (*read_subobject)(const ce_cdb_obj_o0 *reader,
                               uint64_t property,
                               uint64_t defaultt);

    uint64_t (*read_objset_num)(const ce_cdb_obj_o0 *reader,
                                uint64_t property);

    void (*read_objset)(const ce_cdb_obj_o0 *reader,
                        uint64_t property,
                        uint64_t *objs);

};

CE_MODULE(ce_cdb_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_CDB_H
