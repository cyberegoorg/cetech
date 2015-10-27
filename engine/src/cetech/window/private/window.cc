#ifdef CETECH_SDL2

#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "celib/macros.h"
#include "cetech/log_system/log_system.h"
#include "celib/asserts.h"
#include "celib/container/array.inl.h"
#include "celib/crypto/murmur_hash.inl.h"
#include "celib/math/vector2.inl.h"
#include "cetech/application/application.h"


#include "SDL2/SDL.h"

namespace cetech {
    namespace window_internal {
        uint32_t sdl_pos(const uint32_t pos) {
            switch (pos) {
            case window::WINDOWPOS_CENTERED:
                return SDL_WINDOWPOS_CENTERED;

            case window::WINDOWPOS_UNDEFINED:
                return SDL_WINDOWPOS_UNDEFINED;

            default:
                return pos;
            }
        }

        uint32_t sdl_flags(const window::WindowFlags flags) {
            uint32_t sdl_flags = 0;

            if (flags & window::WINDOW_FULLSCREEN) {
                sdl_flags |= SDL_WINDOW_FULLSCREEN;
            }

            return sdl_flags;
        }
    }

    namespace window {
        Window make_window(const char* title,
                           const int32_t x,
                           const int32_t y,
                           const int32_t width,
                           const int32_t height,
                           WindowFlags flags) {
            Window res;

            res.wnd = SDL_CreateWindow(
                title,
                window_internal::sdl_pos(x), window_internal::sdl_pos(y),
                width, height,
                window_internal::sdl_flags(flags)
                );

            if (res.wnd == nullptr) {
                log::error("sys", "Could not create window: %s", SDL_GetError());
            }

            return res;
        }

        Window make_from(void* hndl) {
            Window res;

            res.wnd = SDL_CreateWindowFrom(hndl);

            if (res.wnd == nullptr) {
                log::error("sys", "Could not create window: %s", SDL_GetError());
            }

            return res;
        }


        void destroy_window(const Window& w) {
            SDL_DestroyWindow(w.wnd);
        }

        void set_title(const Window& w, const char* title) {
            SDL_SetWindowTitle(w.wnd, title);
        }

        const char* get_title(const Window& w) {
            return SDL_GetWindowTitle(w.wnd);
        }

        void update(const Window& w) {
            SDL_UpdateWindowSurface(w.wnd);
        }

        void resize(const Window& w, uint32_t width, uint32_t height) {
            SDL_SetWindowSize(w.wnd, width, height);
        }

    }
}

#endif