#ifndef CETECH_BUILDDB_H
#define CETECH_BUILDDB_H

#include <stdint.h>
#include <time.h>

struct ct_resource_id;
struct ce_alloc;

struct ct_builddb_a0 {
    void (*put_file)(const char *filename,
                     time_t mtime);

    void (*put_resource)(const char *fullname,
                         struct ct_resource_id rid,
                         const char *filename,
                         const char *data,
                         uint64_t size);

    void (*set_file_depend)(const char *filename,
                            const char *depend_on);

    bool (*load_cdb_file)(struct ct_resource_id resource,
                          uint64_t object,
                          struct ce_alloc *allocator);

    int (*get_filename_type_name)(char *filename,
                                  size_t max_len,
                                  uint64_t type,
                                  uint64_t name);


    int (*get_fullname)(char *fullname,
                        size_t max_len,
                        uint64_t type,
                        uint64_t name);

    void (*add_dependency)(const char *who_filename,
                           const char *depend_on_filename);

    int (*need_compile)(const char *filename);

    int (*get_resource_dirs)(char ***filename,
                             struct ce_alloc *alloc);

    void (*get_resource_dirs_clean)(char **filename,
                                    struct ce_alloc *alloc);

    int (*get_resource_from_dirs)(const char* dir, char ***filename,
                             struct ce_alloc *alloc);

    void (*get_resource_from_dirs_clean)(char **filename,
                                    struct ce_alloc *alloc);
};

CE_MODULE(ct_builddb_a0);

#endif //CETECH_BUILDDB_H
