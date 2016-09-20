#ifndef CETECH_VIO_H
#define CETECH_VIO_H

#include <stdint.h>
#include "../types.h"
#include "../memory/types.h"
#include "vio_types.h"

struct vio *vio_from_file(const char *path,
                          enum open_mode mode,
                          struct allocator *allocator);

int vio_close(struct vio *file);

int64_t vio_seek(struct vio *file,
                 int64_t offset,
                 enum vio_seek whence);

void vio_seek_to_end(struct vio *file);

i64 vio_skip(struct vio *,
             i64 bytes);

i64 vio_position(struct vio *file);

i64 vio_size(struct vio *file);

size_t vio_read(struct vio *file,
                void *buffer,
                size_t size,
                size_t maxnum);

size_t vio_write(struct vio *file,
                 const void *buffer,
                 size_t size,
                 size_t num);

#endif //CETECH_VIO_H
