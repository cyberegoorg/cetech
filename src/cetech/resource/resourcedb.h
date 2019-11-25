#ifndef CETECH_BUILDDB_H
#define CETECH_BUILDDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>

#define CT_BUILDDB_A0_STR "ct_resourcedb_a0"

#define CT_BUILDDB_API \
   CE_ID64_0("ct_resourcedb_a0", 0xc81a8d961ebf878ULL)

typedef struct ce_cdb_uuid_t0 ct_resource_id_t0;
typedef struct ce_alloc_t0 ce_alloc_t0;

struct ct_resourcedb_a0 {
    int64_t (*get_file_mtime)(const char *filename);

    void (*put_file)(const char *filename,
                     time_t mtime);

    void (*put_resource)(ce_cdb_uuid_t0 rid,
                         const char *type,
                         const char *filename);

    void (*put_resource_blob)(ce_cdb_uuid_t0 rid,
                              const char *data,
                              uint64_t size);

    uint64_t (*load_cdb_file)(ce_cdb_t0 db,
                              ce_cdb_uuid_t0 resource,
                              ce_alloc_t0 *allocator);

    bool (*obj_exist)(ce_cdb_uuid_t0 resource);

    uint64_t (*get_resource_type)(ce_cdb_uuid_t0 resource);

    bool (*get_resource_filename)(ce_cdb_uuid_t0 resource,
                                  char *filename,
                                  size_t max_len);

    ce_cdb_uuid_t0 (*get_file_resource)(const char *filename);

    int (*list_resource_from_dirs)(const char *dir,
                                   char ***filename,
                                   ce_alloc_t0 *alloc);

    int (*list_resource_by_type)(const char *name,
                                 const char *type,
                                 char ***filename,
                                 ce_alloc_t0 *alloc);

    void (*clean_resource_list)(char **filename,
                                ce_alloc_t0 *alloc);

    ce_cdb_uuid_t0 (*get_resource_root)(ce_cdb_uuid_t0 uuid);

    void (*set_resource_root)(ce_cdb_uuid_t0 uuid,
                              ce_cdb_uuid_t0 root_uid);


    ////
    bool (*put_obj)(ce_cdb_t0 db,
                    uint64_t obj,
                    ce_alloc_t0 *alloc);
};

#ifdef __cplusplus
};
#endif

#endif //CETECH_BUILDDB_H
