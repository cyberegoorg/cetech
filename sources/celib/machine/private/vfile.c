#include "celib/containers/eventstream.h"
#include "celib/machine/types.h"

#define LOG_WHERE "vfile"

int vfile_close(struct vfile *file) {
    CE_ASSERT(LOG_WHERE, file != NULL);

    return file->close(file);
}


int64_t vfile_seek(struct vfile *file, int64_t offset, enum vfile_seek whence) {
    CE_ASSERT(LOG_WHERE, file != NULL);

    return file->seek(file, offset, whence);
}

void vfile_seek_to_end(struct vfile *file) {
    CE_ASSERT(LOG_WHERE, file != NULL);

    vfile_seek(file, 0, VFILE_SEEK_END);
}

int64_t vfile_skip(struct vfile *file, i64 bytes) {
    CE_ASSERT(LOG_WHERE, file != NULL);

    return vfile_seek(file, bytes, VFILE_SEEK_CUR);
}

int64_t vfile_position(struct vfile *file) {
    CE_ASSERT(LOG_WHERE, file != NULL);

    return vfile_skip(file, 0);
}

int64_t vfile_size(struct vfile *file) {
    CE_ASSERT(LOG_WHERE, file != NULL);

    return file->size(file);
};

size_t vfile_read(struct vfile *file,
                  void *buffer,
                  size_t size,
                  size_t maxnum) {

    CE_ASSERT(LOG_WHERE, file != NULL);

    return file->read(file, buffer, size, maxnum);
};

size_t vfile_write(struct vfile *file,
                   const void *buffer,
                   size_t size, size_t num) {

    CE_ASSERT(LOG_WHERE, file != NULL);

    return file->write(file, buffer, size, num);
};
