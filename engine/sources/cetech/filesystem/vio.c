#include <stddef.h>

#include <cetech/allocator.h>
#include <cetech/vio.h>

#include "cetech/types.h"
#include "cetech/allocator.h"
#include "cetech/errors.h"

struct vio *vio_from_file(const char *path,
                              enum vio_open_mode mode,
                              struct allocator *allocator);

int vio_close(struct vio *file) {
    CEL_ASSERT("vio", file != NULL);

    return file->close(file);
}


int64_t vio_seek(struct vio *file,
                     int64_t offset,
                     enum vio_seek whence) {
    CEL_ASSERT("vio", file != NULL);

    return file->seek(file, offset, whence);
}

void vio_seek_to_end(struct vio *file) {
    CEL_ASSERT("vio", file != NULL);

    vio_seek(file, 0, VIO_SEEK_END);
}

int64_t vio_skip(struct vio *file,
                     int64_t bytes) {
    CEL_ASSERT("vio", file != NULL);

    return vio_seek(file, bytes, VIO_SEEK_CUR);
}

int64_t vio_position(struct vio *file) {
    CEL_ASSERT("vio", file != NULL);

    return vio_skip(file, 0);
}

int64_t vio_size(struct vio *file) {
    CEL_ASSERT("vio", file != NULL);

    return file->size(file);
};

size_t vio_read(struct vio *file,
                    void *buffer,
                    size_t size,
                    size_t maxnum) {

    CEL_ASSERT("vio", file != NULL);

    return file->read(file, buffer, size, maxnum);
};

size_t vio_write(struct vio *file,
                     const void *buffer,
                     size_t size,
                     size_t num) {

    CEL_ASSERT("vio", file != NULL);

    return file->write(file, buffer, size, num);
};