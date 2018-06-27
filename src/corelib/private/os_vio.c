#include <corelib/os.h>
#include <corelib/module.h>
#include <corelib/api_system.h>
#include <corelib/log.h>
#include <corelib/macros.h>
#include <corelib/memory.h>

#include "include/SDL2/SDL.h"
#include "corelib/allocator.h"

#define LOG_WHERE_OS "vio_sdl"

int64_t vio_sdl_seek(struct ct_vio *file,
                     int64_t offset,
                     enum ct_vio_seek whence) {
    CETECH_ASSERT(LOG_WHERE_OS, file != NULL);

    static int _whence[3] = {
            [VIO_SEEK_SET] = RW_SEEK_SET,
            [VIO_SEEK_CUR] = RW_SEEK_CUR,
            [VIO_SEEK_END] = RW_SEEK_END
    };

    return SDL_RWseek((SDL_RWops *) file->inst, offset, -_whence[whence]);
}

size_t vio_sdl_read(struct ct_vio *file,
                    void *buffer,
                    size_t size,
                    size_t maxnum) {
    CETECH_ASSERT(LOG_WHERE_OS, file != NULL);

    return SDL_RWread((SDL_RWops *) file->inst, buffer, size, maxnum);
};

size_t vio_sdl_write(struct ct_vio *file,
                     const void *buffer,
                     size_t size,
                     size_t maxnum) {
    CETECH_ASSERT(LOG_WHERE_OS, file != NULL);

    return SDL_RWwrite((SDL_RWops *) file->inst, buffer, size, maxnum);
};

int64_t vio_sdl_size(struct ct_vio *file) {
    CETECH_ASSERT(LOG_WHERE_OS, file != NULL);

    return SDL_RWsize((SDL_RWops *) file->inst);
};

int vio_sdl_close(struct ct_vio *file) {
    CETECH_ASSERT(LOG_WHERE_OS, file != NULL);

    SDL_RWclose((SDL_RWops *) file->inst);
    return 1;
}


struct ct_vio *vio_from_file(const char *path,
                             enum ct_vio_open_mode mode) {

    struct ct_alloc *alloc = ct_core_allocator_a0->alloc;

    struct ct_vio *vio = CT_ALLOC(alloc,
                                  struct ct_vio,
                                  sizeof(struct ct_vio));

    CETECH_ASSERT(LOG_WHERE_OS, vio != NULL);

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

struct ct_vio_a0 vio_api = {
        .from_file = vio_from_file,
};

struct ct_vio_a0 *ct_vio_a0 = &vio_api;

