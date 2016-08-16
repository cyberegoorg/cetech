#include <celib/memory/memory.h>
#include "include/SDL2/SDL.h"

#include "celib/errors/errors.h"
#include "../types.h"


#define LOG_WHERE "vfile_sdl"

struct sdl_vfile {
    struct vfile interface;
    struct allocator* allocator;
    SDL_RWops* rw;
};


int64_t vfile_sdl_seek(struct vfile* file, int64_t offset, enum vfile_seek whence) {
    CE_ASSERT(LOG_WHERE, file != NULL);

    static enum vfile_seek _whence[3] = {
        [VFILE_SEEK_SET] = RW_SEEK_SET,
        [VFILE_SEEK_CUR] = RW_SEEK_CUR,
        [VFILE_SEEK_END] = RW_SEEK_END
    };

    struct sdl_vfile *vf = (struct sdl_vfile *) file;

    return SDL_RWseek(vf->rw, offset, -_whence[whence]);
}

size_t vfile_sdl_read(struct vfile* file,
                      void *buffer,
                      size_t size, size_t maxnum) {
    CE_ASSERT(LOG_WHERE, file != NULL);
    struct sdl_vfile *vf = (struct sdl_vfile *) file;

    return SDL_RWread(vf->rw, buffer, size, maxnum);
};

size_t vfile_sdl_write(struct vfile* file,
                   const void *buffer,
                   size_t size, size_t maxnum) {
    CE_ASSERT(LOG_WHERE, file != NULL);
    struct sdl_vfile *vf = (struct sdl_vfile *) file;

    return SDL_RWwrite(vf->rw, buffer, size, maxnum);
};

int64_t vfile_sdl_size(struct vfile* file) {
    CE_ASSERT(LOG_WHERE, file != NULL);
    struct sdl_vfile *vf = (struct sdl_vfile *) file;

    return SDL_RWsize(vf->rw);
};

int vfile_sdl_close(struct vfile* file) {
    CE_ASSERT(LOG_WHERE, file != NULL);
    struct sdl_vfile *vf = (struct sdl_vfile *) file;

    SDL_RWclose(vf->rw);

    allocator_deallocate(vf->allocator, vf);

    return 1;
}


struct vfile* vfile_from_file(const char *path, enum open_mode mode, struct allocator* allocator) {
    struct sdl_vfile *vf = CE_ALLOCATE(allocator, struct sdl_vfile, sizeof(struct sdl_vfile));
    CE_ASSERT(LOG_WHERE, vf != NULL);

    if(!vf) {
        return NULL;
    }

    SDL_RWops *rwops = SDL_RWFromFile(path, mode == VFILE_OPEN_WRITE ? "w" : "r");

    if(!rwops) {
        return NULL;
    }

    vf->interface.write = vfile_sdl_write;
    vf->interface.read = vfile_sdl_read;
    vf->interface.seek = vfile_sdl_seek;
    vf->interface.size = vfile_sdl_size;
    vf->interface.close = vfile_sdl_close;
    vf->allocator = allocator;
    vf->rw = rwops;

    return (struct vfile*) vf;
}