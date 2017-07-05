#include "include/SDL2/SDL.h"


#include <cetech/kernel/log.h>
#include <cetech/kernel/errors.h>
#include <cetech/celib/allocator.h>
#include <cetech/kernel/os.h>

#define LOG_WHERE "vio_sdl"

struct sdl_vio {
    struct allocator *allocator;
    SDL_RWops *rw;
};

int vio_close(struct os_vio *file) {
    CETECH_ASSERT("vio", file != NULL);

    return file->close(file->inst);
}

int64_t vio_seek(struct os_vio *file,
                 int64_t offset,
                 enum vio_seek whence) {
    CETECH_ASSERT("vio", file != NULL);

    return file->seek(file->inst, offset, whence);
}

void vio_seek_to_end(struct os_vio *file) {
    CETECH_ASSERT("vio", file != NULL);

    vio_seek(file, 0, VIO_SEEK_END);
}

int64_t vio_skip(struct os_vio *file,
                 int64_t bytes) {
    CETECH_ASSERT("vio", file != NULL);

    return vio_seek(file, bytes, VIO_SEEK_CUR);
}

int64_t vio_position(struct os_vio *file) {
    CETECH_ASSERT("vio", file != NULL);

    return vio_skip(file, 0);
}

int64_t vio_size(struct os_vio *file) {
    CETECH_ASSERT("vio", file != NULL);

    return file->size(file->inst);
};

size_t vio_read(struct os_vio *file,
                void *buffer,
                size_t size,
                size_t maxnum) {

    CETECH_ASSERT("vio", file != NULL);

    return file->read(file->inst, buffer, size, maxnum);
};

size_t vio_write(struct os_vio *file,
                 const void *buffer,
                 size_t size,
                 size_t num) {

    CETECH_ASSERT("vio", file != NULL);

    return file->write(file->inst, buffer, size, num);
};


int64_t vio_sdl_seek( os_vio_instance_v0 *file,
                     int64_t offset,
                     enum vio_seek whence) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);

    static int _whence[3] = {
            [VIO_SEEK_SET] = RW_SEEK_SET,
            [VIO_SEEK_CUR] = RW_SEEK_CUR,
            [VIO_SEEK_END] = RW_SEEK_END
    };

    return SDL_RWseek((SDL_RWops*)file, offset, -_whence[whence]);
}

size_t vio_sdl_read( os_vio_instance_v0 *file,
                    void *buffer,
                    size_t size,
                    size_t maxnum) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);
    struct sdl_vio *vf = (struct sdl_vio *) file;

    return SDL_RWread((SDL_RWops*)file, buffer, size, maxnum);
};

size_t vio_sdl_write( os_vio_instance_v0 *file,
                     const void *buffer,
                     size_t size,
                     size_t maxnum) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);
    struct sdl_vio *vf = (struct sdl_vio *) file;

    return SDL_RWwrite((SDL_RWops*)file, buffer, size, maxnum);
};

int64_t vio_sdl_size( os_vio_instance_v0 *file) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);
    struct sdl_vio *vf = (struct sdl_vio *) file;

    return SDL_RWsize((SDL_RWops*)file);
};

int vio_sdl_close(os_vio_instance_v0 *file) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);
    struct sdl_vio *vf = (struct sdl_vio *) file;

    SDL_RWclose((SDL_RWops*)file);

//    CETECH_FREE(vf->allocator, vf);

    return 1;
}


struct os_vio *vio_from_file(const char *path,
                             enum vio_open_mode mode,
                             struct allocator *allocator) {

    os_vio *vio = CETECH_ALLOCATE(allocator, os_vio, sizeof(os_vio));
    sdl_vio *vf = CETECH_ALLOCATE(allocator, sdl_vio, sizeof(struct sdl_vio));

    CETECH_ASSERT(LOG_WHERE, vf != NULL);

    if (!vf) {
        return NULL;
    }

    SDL_RWops *rwops = SDL_RWFromFile(path, mode == VIO_OPEN_WRITE ? "w" : "r");

    if (!rwops) {
        return NULL;
    }

    vf->allocator = allocator;
    vf->rw = rwops;

    vio->inst = vf;
    vio->write = vio_sdl_write;
    vio->read = vio_sdl_read;
    vio->seek = vio_sdl_seek;
    vio->size = vio_sdl_size;
    vio->close = vio_sdl_close;


    return vio;
}