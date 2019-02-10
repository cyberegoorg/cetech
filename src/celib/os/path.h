//                          **OS path**
//

#ifndef CE_OS_PATH_H
#define CE_OS_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#define CE_OS_PATH_API \
    CE_ID64_0("ce_os_path_a0", 0xb1f6bb5e13c8aca1ULL)

typedef struct ce_alloc_t0 ce_alloc_t0;

struct ce_os_path_a0 {
    // Get file modified time
    uint32_t (*file_mtime)(const char *path);

    // List dir
    // - path Dir path
    // - recursive Resucrsive list?
    // - files Result files
    // - allocator Allocator
    void (*list)(const char *path,
                 const char **patern,
                 uint32_t patern_n,
                 int recursive,
                 int only_dir,
                 char ***files,
                 uint32_t *count,
                 struct ce_alloc_t0 *allocator);

    // Free list dir array
    // - files Files array
    // - allocator Allocator
    void (*list_free)(char **files,
                      uint32_t count,
                      struct ce_alloc_t0 *allocator);

    // Create dir path
    // - path Path
    int (*make_path)(const char *path);

    // Get filename from path
    // - path Path
    const char *(*filename)(const char *path);

    // Get file basename (filename without extension)
    // - path Path
    // - out Out basename
    // - size
    void (*basename)(const char *path,
                     char *out);

    void (*dir)(char *out,
                const char *path);

    void (*dirname)(char *out,
                    const char *path);

    // Get file extension
    // - path Path
    const char *(*extension)(const char *path);

    // Join paths and return path len.
    // - allocator Allocator
    // - count Path count.
    void (*join)(char **buffer,
                 struct ce_alloc_t0 *allocator,
                 uint32_t count,
                 ...);

    void (*copy_file)(ce_alloc_t0 *allocator,
                      const char *from,
                      const char *to);

    bool (*is_dir)(const char *path);
};

CE_MODULE(ce_os_path_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_OS_PATH_H
