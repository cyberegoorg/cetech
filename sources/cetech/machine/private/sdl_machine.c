#include <include/SDL2/SDL.h>
#include <celib/containers/array.h>
#include <celib/containers/eventstream.h>
#include <cetech/application/application.h>

#define LOG_WHERE "machine.sdl"

void machine_process_impl(struct eventstream *stream) {
    SDL_Event e;

    while (SDL_PollEvent(&e) > 0) {
        switch (e.type) {
            case SDL_QUIT:
                application_quit();
                break;

            default:
                break;
        }
    }
}

int machine_init_impl() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        log_error(LOG_WHERE, "Could not init sdl - %s", SDL_GetError());
        return 0;
    }
    return 1;
}

void machine_shutdown_impl() {
    SDL_Quit();
}
