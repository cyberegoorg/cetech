#ifndef CETECH_VIO_H
#define CETECH_VIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

//==============================================================================
// File Interface
//==============================================================================

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

#ifdef __cplusplus
}
#endif

#endif //CETECH_VIO_H
