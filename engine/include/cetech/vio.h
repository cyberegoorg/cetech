#ifndef CETECH_VIO_H
#define CETECH_VIO_H

//==============================================================================
// Enums
//==============================================================================

#include <stddef.h>
#include <stdint.h>

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

struct vio *vio_from_file(const char *path,
                              enum vio_open_mode mode,
                              struct allocator *allocator);

int vio_close(struct vio *file);


int64_t vio_seek(struct vio *file,
                     int64_t offset,
                     enum vio_seek whence);

void vio_seek_to_end(struct vio *file);

int64_t vio_skip(struct vio *file,
                     int64_t bytes);

int64_t vio_position(struct vio *file);

int64_t vio_size(struct vio *file);

size_t vio_read(struct vio *file,
                    void *buffer,
                    size_t size,
                    size_t maxnum);

size_t vio_write(struct vio *file,
                     const void *buffer,
                     size_t size,
                     size_t num);

#endif //CETECH_VIO_H
