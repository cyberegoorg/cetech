#include "../vfile.h"

#include "include/SDL2/SDL.h"
#include "../../errors/errors.h"

vfile_t vfile_from_file(const char *path, enum open_mode mode) {
    SDL_RWops *rwops = SDL_RWFromFile(path, mode == VFILE_WRITE ? "w" : "r");

    CE_ASSERT_MSG("File_SDL", rwops != NULL, "%s", SDL_GetError());

    return rwops;
}

void vfile_close(vfile_t file) {
    CE_ASSERT("File_SDL", file != NULL);
    SDL_RWops *rwops = (SDL_RWops *) file;

    SDL_RWclose(rwops);
}


void vfile_seek(vfile_t file, size_t position) {
    CE_ASSERT("File_SDL", file != NULL);
    SDL_RWops *rwops = (SDL_RWops *) file;

    SDL_RWseek(rwops, position, RW_SEEK_SET);
}

void vfile_seek_to_end(vfile_t file) {
    CE_ASSERT("File_SDL", file != NULL);
    SDL_RWops *rwops = (SDL_RWops *) file;

    SDL_RWseek(rwops, 0, RW_SEEK_END);
}

void vfile_skip(vfile_t file, size_t bytes) {
    CE_ASSERT("File_SDL", file != NULL);
    SDL_RWops *rwops = (SDL_RWops *) file;

    SDL_RWseek(rwops, bytes, RW_SEEK_CUR);
}

size_t vfile_position(vfile_t file) {
    CE_ASSERT("File_SDL", file != NULL);
    SDL_RWops *rwops = (SDL_RWops *) file;

    return SDL_RWtell(rwops);
}

size_t vfile_size(vfile_t file) {
    CE_ASSERT("File_SDL", file != NULL);

    const size_t curent_pos = vfile_position(file);
    vfile_seek_to_end(file);

    const size_t sz = vfile_position(file);
    vfile_seek(file, curent_pos);

    return sz;
};

size_t vfile_read(vfile_t file,
                  char *buffer,
                  size_t size) {
    CE_ASSERT("File_SDL", file != NULL);
    SDL_RWops *rwops = (SDL_RWops *) file;

    return SDL_RWread(rwops, buffer, sizeof(char), size);
};

size_t vfile_write(vfile_t file,
                   const char *buffer,
                   size_t size) {
    CE_ASSERT("File_SDL", file != NULL);
    SDL_RWops *rwops = (SDL_RWops *) file;

    return SDL_RWwrite(rwops, buffer, sizeof(char), size);
};