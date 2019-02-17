#ifndef CE_FILESYSTEM_TYPES_H
#define CE_FILESYSTEM_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib_types.h"

#define CE_FS_API \
    CE_ID64_0("ce_fs_a0", 0xb833e93306ececb0ULL)

typedef struct ce_alloc_t0 ce_alloc_t0;
typedef struct ce_vio_t0 ce_vio_t0;

typedef enum ce_fs_open_mode_e0 {
    FS_OPEN_READ,
    FS_OPEN_WRITE,
} ce_fs_open_mode_e0;

//! Filesystem API V0
struct ce_fs_a0 {
    struct ce_vio_t0 *(*open)(uint64_t root,
                           const char *path,
                           ce_fs_open_mode_e0 mode);

    void (*map_root_dir)(uint64_t root,
                         const char *base_path,
                         bool watch);

    void (*close)(ce_vio_t0 *file);

    void (*listdir)(uint64_t root,
                    const char *path,
                    const char *filter,
                    bool only_dir,
                    bool recursive,
                    char ***files,
                    uint32_t *count,
                    ce_alloc_t0 *allocator);

    void (*listdir_iter)(uint64_t root,
                         const char *path,
                         const char *filter,
                         bool only_dir,
                         bool recursive,
                         void (*on_item)(const char *path));

    void (*listdir_free)(char **files,
                         uint32_t count,
                         ce_alloc_t0 *allocator);

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

#ifdef __cplusplus
};
#endif

#endif //CE_FILESYSTEM_TYPES_H