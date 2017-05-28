#ifndef CETECH_PATH_H
#define CETECH_PATH_H

#include "cetech/core/container/array.inl"

//==============================================================================
// File Interface
//==============================================================================

struct path_v0 {
    //! Get file modified time
    //! \param path File path
    //! \return Modified time
    uint32_t (*file_mtime)(const char *path);

    //! List dir
    //! \param path Dir path
    //! \param recursive Resucrsive list?
    //! \param files Result files
    //! \param allocator Allocator
    void (*dir_list)(const char *path,
                     int recursive,
                     struct array_pchar *files,
                     struct allocator *allocator);

    //! Free list dir array
    //! \param files Files array
    //! \param allocator Allocator
    void (*dir_list_free)(struct array_pchar *files,
                          struct allocator *allocator);

    //! Create dir path
    //! \param path Path
    //! \return 1 of ok else 0
    int (*dir_make_path)(const char *path);

    //! Get filename from path
    //! \param path Path
    //! \return Filename
    const char *(*path_filename)(const char *path);

    //! Get file basename (filename without extension)
    //! \param path Path
    //! \param out Out basename
    //! \param size
    void (*path_basename)(const char *path,
                          char *out,
                          size_t size);

    void (*path_dir)(char *out,
                     size_t size,
                     const char *path);

    //! Get file extension
    //! \param path Path
    //! \return file extension
    const char *(*path_extension)(const char *path);

    //! Join path
    //! \param result Output path
    //! \param maxlen Result len
    //! \param base_path Base path
    //! \param path Path
    //! \return Result path len
    int64_t (*path_join)(char *result,
                         uint64_t maxlen,
                         const char *base_path,
                         const char *path);
};

#endif //CETECH_PATH_H
