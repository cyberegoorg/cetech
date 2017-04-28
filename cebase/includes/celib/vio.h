#ifndef CELIB_VIO_H
#define CELIB_VIO_H

#include <memory.h>

#include "types.h"

//==============================================================================
// Enums
//==============================================================================


enum cel_vio_open_mode {
    VIO_OPEN_READ,
    VIO_OPEN_WRITE,
};

enum cel_vio_seek {
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
                enum cel_vio_seek whence);

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

struct vio *cel_vio_from_file(const char *path,
                              enum cel_vio_open_mode mode,
                              struct cel_allocator *allocator);

int cel_vio_close(struct vio *file);


int64_t cel_vio_seek(struct vio *file,
                     int64_t offset,
                     enum cel_vio_seek whence);

void cel_vio_seek_to_end(struct vio *file);

int64_t cel_vio_skip(struct vio *file,
                     int64_t bytes);

int64_t cel_vio_position(struct vio *file);

int64_t cel_vio_size(struct vio *file);

size_t cel_vio_read(struct vio *file,
                    void *buffer,
                    size_t size,
                    size_t maxnum);

size_t cel_vio_write(struct vio *file,
                     const void *buffer,
                     size_t size,
                     size_t num);

#endif //CELIB_VIO_H
