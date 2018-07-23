#ifndef CETECH_FILESYSTEM_TYPES_H
#define CETECH_FILESYSTEM_TYPES_H



//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <stdbool.h>

#include <corelib/module.inl>

struct ct_alloc;

enum ct_fs_open_mode {
    FS_OPEN_READ,
    FS_OPEN_WRITE,
};

//==============================================================================
// Api
//==============================================================================

//! Filesystem API V0
struct ct_fs_a0 {
    struct ct_vio *(*open)(uint64_t root,
                           const char *path,
                           enum ct_fs_open_mode mode);

    void (*map_root_dir)(uint64_t root,
                         const char *base_path,
                         bool watch);

    void (*close)(struct ct_vio *file);

    void (*listdir)(uint64_t root,
                    const char *path,
                    const char *filter,
                    bool only_dir,
                    bool recursive,
                    char ***files,
                    uint32_t *count,
                    struct ct_alloc *allocator);

    void (*listdir_iter)(uint64_t root,
                         const char *path,
                         const char *filter,
                         bool only_dir,
                         bool recursive,
                         void (*on_item)(const char *path));

    void (*listdir_free)(char **files,
                         uint32_t count,
                         struct ct_alloc *allocator);

    int (*create_directory)(uint64_t root,
                            const char *path);

    int64_t (*file_mtime)(uint64_t root,
                          const char *path);

    void (*get_full_path)(uint64_t root,
                          const char *path,
                          char *fullpath,
                          uint32_t max_len);

};

CT_MODULE(ct_fs_a0);

#endif //CETECH_FILESYSTEM_TYPES_H