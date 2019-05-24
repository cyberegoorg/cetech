#ifndef CETECH_BUILDDB_H
#define CETECH_BUILDDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>

#define CT_BUILDDB_API \
   CE_ID64_0("ct_resourcedb_a0", 0xc81a8d961ebf878ULL)

typedef struct ct_resource_id_t0 ct_resource_id_t0;
typedef struct ce_alloc_t0 ce_alloc_t0;

struct ct_resourcedb_a0 {
    void (*put_file)(const char *filename,
                     time_t mtime);

    void (*put_resource)(ct_resource_id_t0 rid,
                         const char *type,
                         const char *filename,
                         const char *name);

    void (*put_resource_blob)(ct_resource_id_t0 rid,
                              const char *data,
                              uint64_t size);

    bool (*load_cdb_file)(ce_cdb_t0 db,
                          ct_resource_id_t0 resource,
                          uint64_t object,
                          ce_alloc_t0 *allocator);

    void (*add_dependency)(const char *who_filename,
                           const char *depend_on_filename);

    int (*need_compile)(const char *filename);


    bool (*obj_exist)(ct_resource_id_t0 resource);

    uint64_t (*get_resource_type)(ct_resource_id_t0 resource);

    bool (*get_resource_filename)(ct_resource_id_t0 resource,
                                  char *filename,
                                  size_t max_len);

    void (*get_resource_by_fullname)(const char *fullname,
                                     ct_resource_id_t0 *resource);

    int (*get_resource_dirs)(char ***filename,
                             ce_alloc_t0 *alloc);

    void (*get_resource_dirs_clean)(char **filename,
                                    ce_alloc_t0 *alloc);

    int (*get_resource_from_dirs)(const char *dir,
                                  char ***filename,
                                  ce_alloc_t0 *alloc);

    void (*get_resource_from_dirs_clean)(char **filename,
                                         ce_alloc_t0 *alloc);

    uint64_t (*get_uid)(const char *name,
                        const char *type);

    int (*get_resource_by_type)(const char *name,
                                const char *type,
                                char ***filename,
                                ce_alloc_t0 *alloc);

    void (*get_resource_by_type_clean)(char **filename,
                                       ce_alloc_t0 *alloc);
};

CE_MODULE(ct_resourcedb_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_BUILDDB_H
