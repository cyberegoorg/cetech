
#include <celib/module.h>
#include <celib/api.h>
#include <celib/log.h>
#include <celib/macros.h>
#include <celib/memory/memory.h>

#include "include/SDL2/SDL.h"
#include "celib/memory/allocator.h"
#include <celib/os/vio.h>

#define LOG_WHERE_OS "vio_sdl"

int64_t vio_sdl_seek(ce_vio_t0 *file,
                     int64_t offset,
                     enum ce_vio_seek whence) {
    CE_ASSERT(LOG_WHERE_OS, file != NULL);

    static int _whence[] = {
            [VIO_SEEK_SET] = RW_SEEK_SET,
            [VIO_SEEK_CUR] = RW_SEEK_CUR,
            [VIO_SEEK_END] = RW_SEEK_END
    };

    return SDL_RWseek((SDL_RWops *) file->inst, offset, _whence[whence]);
}

size_t vio_sdl_read(ce_vio_t0 *file,
                    void *buffer,
                    size_t size,
                    size_t maxnum) {
    CE_ASSERT(LOG_WHERE_OS, file != NULL);

    return SDL_RWread((SDL_RWops *) file->inst, buffer, size, maxnum);
};

size_t vio_sdl_write(ce_vio_t0 *file,
                     const void *buffer,
                     size_t size,
                     size_t maxnum) {
    CE_ASSERT(LOG_WHERE_OS, file != NULL);

    return SDL_RWwrite((SDL_RWops *) file->inst, buffer, size, maxnum);
};

int64_t vio_sdl_size(ce_vio_t0 *file) {
    CE_ASSERT(LOG_WHERE_OS, file != NULL);

    return SDL_RWsize((SDL_RWops *) file->inst);
};

int vio_sdl_close(ce_vio_t0 *file) {
    CE_ASSERT(LOG_WHERE_OS, file != NULL);

    SDL_RWclose((SDL_RWops *) file->inst);
    return 1;
}


struct ce_vio_t0 *vio_from_file(const char *path,
                             enum ce_vio_open_mode mode) {

    struct ce_alloc_t0 *alloc =ce_memory_a0->system;

    struct ce_vio_t0 *vio = CE_ALLOC(alloc,
                                  struct ce_vio_t0,
                                  sizeof(ce_vio_t0));

    CE_ASSERT(LOG_WHERE_OS, vio != NULL);

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

struct ce_os_vio_a0 vio_api = {
        .from_file = vio_from_file,
};

struct ce_os_vio_a0 *ce_os_vio_a0 = &vio_api;

