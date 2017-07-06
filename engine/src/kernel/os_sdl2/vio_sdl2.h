#include "include/SDL2/SDL.h"
#include "../allocator_core_private.h"


#include <cetech/kernel/log.h>
#include <cetech/kernel/errors.h>
#include <cetech/celib/allocator.h>
#include <cetech/kernel/os.h>

#define LOG_WHERE "vio_sdl"

int64_t vio_sdl_seek(ct_vio_instance *file,
                     int64_t offset,
                     enum ct_vio_seek whence) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);

    static int _whence[3] = {
            [VIO_SEEK_SET] = RW_SEEK_SET,
            [VIO_SEEK_CUR] = RW_SEEK_CUR,
            [VIO_SEEK_END] = RW_SEEK_END
    };

    return SDL_RWseek((SDL_RWops*)file, offset, -_whence[whence]);
}

size_t vio_sdl_read(ct_vio_instance *file,
                    void *buffer,
                    size_t size,
                    size_t maxnum) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);

    return SDL_RWread((SDL_RWops*) file, buffer, size, maxnum);
};

size_t vio_sdl_write(ct_vio_instance *file,
                     const void *buffer,
                     size_t size,
                     size_t maxnum) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);

    return SDL_RWwrite((SDL_RWops*) file, buffer, size, maxnum);
};

int64_t vio_sdl_size(ct_vio_instance *file) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);

    return SDL_RWsize((SDL_RWops*) file);
};

int vio_sdl_close(ct_vio_instance *file) {
    CETECH_ASSERT(LOG_WHERE, file != NULL);

    SDL_RWclose((SDL_RWops*) file);
    return 1;
}


struct ct_vio *vio_from_file(const char *path,
                             enum ct_vio_open_mode mode) {

    ct_vio *vio = CETECH_ALLOCATE(core_allocator::get(), ct_vio, sizeof(ct_vio));

    CETECH_ASSERT(LOG_WHERE, vio != NULL);

    if (!vio) {
        return NULL;
    }

    SDL_RWops *rwops = SDL_RWFromFile(path, mode == VIO_OPEN_WRITE ? "w" : "r");

    if (!rwops) {
        return NULL;
    }

    vio->inst = rwops;
    vio->write = vio_sdl_write;
    vio->read = vio_sdl_read;
    vio->seek = vio_sdl_seek;
    vio->size = vio_sdl_size;
    vio->close = vio_sdl_close;

    return vio;
}