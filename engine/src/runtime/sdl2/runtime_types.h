#pragma once

struct SDL_Window;
struct SDL_RWops;

namespace cetech {
    struct Window {
        SDL_Window* wnd;
    };

    struct File {
        SDL_RWops* ops;
    };
}
