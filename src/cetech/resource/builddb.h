#ifndef CETECH_BUILDDB_H
#define CETECH_BUILDDB_H

#include <stdint.h>

struct ct_resource_id;

struct ct_builddb_a0 {
    void (*put_file)(const char *filename,
                     time_t mtime,
                     const char *data,
                     uint64_t size);

    void (*put_resource)(struct ct_resource_id resource,
                         const char *filename);

    void (*set_file_depend)(const char *filename,
                            const char *depend_on);

    bool (*load_cdb_file)(struct ct_resource_id resource,
                          uint64_t object,
                          struct ce_alloc *allocator);

    int (*get_filename_type_name)(char *filename,
                                  size_t max_len,
                                  uint64_t type,
                                  uint64_t name);

    void (*add_dependency)(const char *who_filename,
                           const char *depend_on_filename);

    int (*need_compile)(const char *filename);
};

CE_MODULE(ct_builddb_a0);

#endif //CETECH_BUILDDB_H
