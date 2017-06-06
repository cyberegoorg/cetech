//! \defgroup Filesystem
//! Filesystem
//! \{
#ifndef CETECH_FILESYSTEM_TYPES_H
#define CETECH_FILESYSTEM_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

enum fs_open_mode {
    FS_OPEN_READ,
    FS_OPEN_WRITE,
};

//==============================================================================
// Api
//==============================================================================

//! Filesystem API V0
struct filesystem_api_v0 {
    //! Return root dir
    //! \param root Root
    //! \return Root dir or NULL if root is invalid.
    const char *(*root_dir)(uint64_t root);

    //! Open file
    //! \param root Root
    //! \param path File path
    //! \param mode Open mode
    //! \return File or NULL
    struct vio *(*open)(uint64_t root,
                        const char *path,
                        enum fs_open_mode mode);

    //! Map path for root
    //! \param root Root
    //! \param base_path Path
    void (*map_root_dir)(uint64_t root,
                         const char *base_path);

    //! Close file
    //! \param file file
    void (*close)(struct vio *file);

    //! List directory
    //! \param root Root
    //! \param path Dir path
    //! \param files Output file array
    //! \param allocator Allocator
    void (*listdir)(uint64_t root,
                    const char *path,
                    const char *filter,
                    char ***files,
                    uint32_t *count,
                    struct allocator *allocator);

    //! Free list directory array
    //! \param files File array
    //! \param allocator Allocator
    void (*listdir_free)(char **files,
                         uint32_t count,
                         struct allocator *allocator);

    //! Create directory in root
    //! \param root Root
    //! \param path Directory path
    //! \return 1 if ok else 0
    int (*create_directory)(uint64_t root,
                            const char *path);

    //! Get file modified time
    //! \param root Root
    //! \param path File path
    //! \return Modified time
    int64_t (*file_mtime)(uint64_t root,
                          const char *path);

    //! Get full path
    //! \param root Root
    //! \param result Result path
    //! \param maxlen Max result len
    //! \param filename Filename
    //! \return 1 if ok else 0
    int (*fullpath)(uint64_t root,
                    char *result,
                    uint64_t maxlen,
                    const char *filename);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_FILESYSTEM_TYPES_H
//! |}