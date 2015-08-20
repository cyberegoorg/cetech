#pragma once

#include "SDL2/SDL.h"

#include "common/asserts.h"
#include "runtime/runtime.h"

namespace cetech1 {
    namespace runtime {
        static uint8_t KeyboardStates[512] = { 0 };
        static uint8_t KeyboardStatesLast[512] = { 0 };

        void init() {
            CE_ASSERT(SDL_Init(SDL_INIT_VIDEO) == 0);
        }

        void shutdown() {
            SDL_Quit();
        }

        void frame_start() {
            SDL_Event e;

            while (SDL_PollEvent(&e) > 0) {
                switch (e.type) {
                case SDL_QUIT:
                    exit(0);
                    break;
                }
            }

            memcpy(KeyboardStates, SDL_GetKeyboardState(NULL), 512);
        }

        void frame_end() {
            memcpy(KeyboardStatesLast, KeyboardStates, 512);
        }
    }

    namespace runtime {
        namespace window_internal {
            CE_INLINE uint32_t pos2sdl_pos(const uint32_t pos) {
                switch (pos) {
                case window::WINDOWPOS_CENTERED:
                    return SDL_WINDOWPOS_CENTERED;

                case window::WINDOWPOS_UNDEFINED:
                    return SDL_WINDOWPOS_UNDEFINED;

                default:
                    return pos;
                }
            }

            CE_INLINE uint32_t flags2sdl_flags(const window::WindowFlags flags) {
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
                    window_internal::pos2sdl_pos(x), window_internal::pos2sdl_pos(y),
                    width, height,
                    window_internal::flags2sdl_flags(flags)
                    );

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
        }

        namespace keyboard {
            uint32_t button_index(const char* scancode) {
                return SDL_GetScancodeFromName(scancode);
            }

            const char* button_name(const uint32_t button_index) {
                return SDL_GetScancodeName((SDL_Scancode)button_index);
            }

            bool button_state(const uint32_t button_index) {
                return KeyboardStates[button_index];
            }

            bool button_pressed(const uint32_t button_index) {
                return !KeyboardStatesLast[button_index] && KeyboardStates[button_index];
            }

            bool button_released(const uint32_t button_index) {
                return KeyboardStatesLast[button_index] && !KeyboardStates[button_index];
            }
        };
    }
}
