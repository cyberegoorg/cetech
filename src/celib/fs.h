#ifndef CE_FILESYSTEM_TYPES_H
#define CE_FILESYSTEM_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#include <celib/module.inl>

#define CE_FS_API \
    CE_ID64_0("ce_fs_a0", 0xb833e93306ececb0ULL)

struct ce_alloc;

enum ce_fs_open_mode {
    FS_OPEN_READ,
    FS_OPEN_WRITE,
};

//! Filesystem API V0
struct ce_fs_a0 {
    struct ce_vio *(*open)(uint64_t root,
                           const char *path,
                           enum ce_fs_open_mode mode);

    void (*map_root_dir)(uint64_t root,
                         const char *base_path,
                         bool watch);

    void (*close)(struct ce_vio *file);

    void (*listdir)(uint64_t root,
                    const char *path,
                    const char *filter,
                    bool only_dir,
                    bool recursive,
                    char ***files,
                    uint32_t *count,
                    struct ce_alloc *allocator);

    void (*listdir_iter)(uint64_t root,
                         const char *path,
                         const char *filter,
                         bool only_dir,
                         bool recursive,
                         void (*on_item)(const char *path));

    void (*listdir_free)(char **files,
                         uint32_t count,
                         struct ce_alloc *allocator);

    int (*create_directory)(uint64_t root,
                            const char *path);

    int64_t (*file_mtime)(uint64_t root,
                          const char *path);

    void (*get_full_path)(uint64_t root,
                          const char *path,
                          char *fullpath,
                          uint32_t max_len);

};

CE_MODULE(ce_fs_a0);

#endif //CE_FILESYSTEM_TYPES_H