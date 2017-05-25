#include "include/SDL2/SDL.h"


#include <cetech/kernel/log.h>
#include <cetech/core/errors.h>
#include <cetech/core/allocator.h>
#include <cetech/kernel/fs.h>

#define LOG_WHERE "vio_sdl"

struct vio_sdl {
    struct vio interface;
    struct allocator *allocator;
    SDL_RWops *rw;
};

int64_t vio_sdl_seek(struct vio *file,
                     int64_t offset,
                     enum vio_seek whence) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);

    static enum vio_seek _whence[3] = {
            [VIO_SEEK_SET] = RW_SEEK_SET,
            [VIO_SEEK_CUR] = RW_SEEK_CUR,
            [VIO_SEEK_END] = RW_SEEK_END
    };

    struct vio_sdl *vf = (struct vio_sdl *) file;

    return SDL_RWseek(vf->rw, offset, -_whence[whence]);
}

size_t vio_sdl_read(struct vio *file,
                    void *buffer,
                    size_t size,
                    size_t maxnum) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);
    struct vio_sdl *vf = (struct vio_sdl *) file;

    return SDL_RWread(vf->rw, buffer, size, maxnum);
};

size_t vio_sdl_write(struct vio *file,
                     const void *buffer,
                     size_t size,
                     size_t maxnum) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);
    struct vio_sdl *vf = (struct vio_sdl *) file;

    return SDL_RWwrite(vf->rw, buffer, size, maxnum);
};

int64_t vio_sdl_size(struct vio *file) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);
    struct vio_sdl *vf = (struct vio_sdl *) file;

    return SDL_RWsize(vf->rw);
};

int vio_sdl_close(struct vio *file) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);
    struct vio_sdl *vf = (struct vio_sdl *) file;

    SDL_RWclose(vf->rw);

    allocator_deallocate(vf->allocator, vf);

    return 1;
}


struct vio *vio_from_file(const char *path,
                          enum vio_open_mode mode,
                          struct allocator *allocator) {
    struct vio_sdl *vf =
    CETECH_ALLOCATE(allocator, struct vio_sdl,
                    sizeof(struct vio_sdl));
    CETECH_ASSERT(LOG_WHERE, vf != NULL);

    if (!vf) {
        return NULL;
    }

    SDL_RWops *rwops = SDL_RWFromFile(path, mode == VIO_OPEN_WRITE ? "w" : "r");

    if (!rwops) {
        return NULL;
    }

    vf->interface.write = vio_sdl_write;
    vf->interface.read = vio_sdl_read;
    vf->interface.seek = vio_sdl_seek;
    vf->interface.size = vio_sdl_size;
    vf->interface.close = vio_sdl_close;
    vf->allocator = allocator;
    vf->rw = rwops;

    return (struct vio *) vf;
}