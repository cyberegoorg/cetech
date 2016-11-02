#ifndef CELIB_VIO_H
#define CELIB_VIO_H

#include "../types.h"
#include "../memory/types.h"
#include "vio_types.h"

struct vio *cel_vio_from_file(const char *path,
                              enum cel_vio_open_mode mode,
                              struct cel_allocator *allocator);

static int cel_vio_close(struct vio *file) {
    CEL_ASSERT("vio", file != NULL);

    return file->close(file);
}


static int64_t cel_vio_seek(struct vio *file,
                            int64_t offset,
                            enum cel_vio_seek whence) {
    CEL_ASSERT("vio", file != NULL);

    return file->seek(file, offset, whence);
}

static void cel_vio_seek_to_end(struct vio *file) {
    CEL_ASSERT("vio", file != NULL);

    cel_vio_seek(file, 0, VIO_SEEK_END);
}

static int64_t cel_vio_skip(struct vio *file,
                            i64 bytes) {
    CEL_ASSERT("vio", file != NULL);

    return cel_vio_seek(file, bytes, VIO_SEEK_CUR);
}

static int64_t cel_vio_position(struct vio *file) {
    CEL_ASSERT("vio", file != NULL);

    return cel_vio_skip(file, 0);
}

static int64_t cel_vio_size(struct vio *file) {
    CEL_ASSERT("vio", file != NULL);

    return file->size(file);
};

static size_t cel_vio_read(struct vio *file,
                           void *buffer,
                           size_t size,
                           size_t maxnum) {

    CEL_ASSERT("vio", file != NULL);

    return file->read(file, buffer, size, maxnum);
};

static size_t cel_vio_write(struct vio *file,
                            const void *buffer,
                            size_t size,
                            size_t num) {

    CEL_ASSERT("vio", file != NULL);

    return file->write(file, buffer, size, num);
};

#endif //CELIB_VIO_H
