#ifndef CETECH_BUILDDB_H
#define CETECH_BUILDDB_H

#include <stdint.h>
#include <time.h>

#define CT_BUILDDB_API \
    CE_ID64_0("ct_builddb_a0", 0xe682eb268876882aULL)

struct ct_resource_id;
struct ce_alloc;

struct ct_builddb_a0 {
    void (*put_file)(const char *filename,
                     time_t mtime);

    void (*put_resource)(struct ct_resource_id rid,
                         const char *type,
                         const char *filename,
                         const char *name);

    void (*put_resource_blob)(struct ct_resource_id rid,
                              const char *data,
                              uint64_t size);

    void (*set_file_depend)(const char *filename,
                            const char *depend_on);

    bool (*load_cdb_file)(struct ct_resource_id resource,
                          uint64_t object,
                          struct ce_alloc *allocator);

    void (*add_dependency)(const char *who_filename,
                           const char *depend_on_filename);

    int (*need_compile)(const char *filename);


    bool (*obj_exist)(struct ct_resource_id resource);

    uint64_t (*get_resource_type)(struct ct_resource_id resource);

    uint64_t (*get_resource_filename)(struct ct_resource_id resource,
                                      char *filename,
                                      size_t max_len);

    void (*get_resource_by_fullname)(const char *fullname,
                                     struct ct_resource_id *resource);

    int (*get_resource_dirs)(char ***filename,
                             struct ce_alloc *alloc);

    void (*get_resource_dirs_clean)(char **filename,
                                    struct ce_alloc *alloc);

    int (*get_resource_from_dirs)(const char *dir,
                                  char ***filename,
                                  struct ce_alloc *alloc);

    void (*get_resource_from_dirs_clean)(char **filename,
                                         struct ce_alloc *alloc);
};

CE_MODULE(ct_builddb_a0);

#endif //CETECH_BUILDDB_H
