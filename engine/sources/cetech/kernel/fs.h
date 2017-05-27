#ifndef CETECH_FS_H
#define CETECH_FS_H

#include "cetech/core/array.inl"

//==============================================================================
// File Interface
//==============================================================================

//! Get file modified time
//! \param path File path
//! \return Modified time
uint32_t file_mtime(const char *path);

struct path_v0 {
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

enum vio_open_mode {
    VIO_OPEN_READ,
    VIO_OPEN_WRITE,
};

enum vio_seek {
    VIO_SEEK_SET,
    VIO_SEEK_CUR,
    VIO_SEEK_END
};

//==============================================================================
// Structs
//==============================================================================

struct vio {
    int64_t (*size)(struct vio *vio);

    int64_t (*seek)(struct vio *vio,
                    int64_t offset,
                    enum vio_seek whence);

    size_t (*read)(struct vio *vio,
                   void *ptr,
                   size_t size,
                   size_t maxnum);

    size_t (*write)(struct vio *vio,
                    const void *ptr,
                    size_t size,
                    size_t num);

    int (*close)(struct vio *vio);
};

struct vio_api_v0 {
    struct vio *(*from_file)(const char *path,
                              enum vio_open_mode mode,
                              struct allocator *allocator);

    int (*close)(struct vio *file);


    int64_t (*seek)(struct vio *file,
                     int64_t offset,
                     enum vio_seek whence);

    void (*seek_to_end)(struct vio *file);

    int64_t (*skip)(struct vio *file,
                     int64_t bytes);

    int64_t (*position)(struct vio *file);

    int64_t (*size)(struct vio *file);

    size_t (*read)(struct vio *file,
                    void *buffer,
                    size_t size,
                    size_t maxnum);

    size_t (*write)(struct vio *file,
                     const void *buffer,
                     size_t size,
                     size_t num);

};


#endif //CETECH_FS_H
