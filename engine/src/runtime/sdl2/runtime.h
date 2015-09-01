#pragma once

#include "SDL2/SDL.h"

#include "common/log.h"
#include "common/asserts.h"
#include "common/murmur_hash.h"
#include "common/math/vector2.h"
#include "runtime/runtime.h"

namespace cetech {
    namespace runtime {
        namespace log_internal {
            CE_INLINE void sdl_log_output_function(void* userdata,
                                                   int category,
                                                   SDL_LogPriority priority,
                                                   const char* message) {
                const char* where = nullptr;

                switch (category) {
                case SDL_LOG_CATEGORY_APPLICATION:
                    where = "sdl.app";
                    break;

                case SDL_LOG_CATEGORY_ERROR:
                    where = "sdl.error";
                    break;

                case SDL_LOG_CATEGORY_SYSTEM:
                    where = "sdl.system";
                    break;

                case SDL_LOG_CATEGORY_AUDIO:
                    where = "sdl.audio";
                    break;

                case SDL_LOG_CATEGORY_VIDEO:
                    where = "sdl.video";
                    break;

                case SDL_LOG_CATEGORY_RENDER:
                    where = "sdl.render";
                    break;

                case SDL_LOG_CATEGORY_INPUT:
                    where = "sdl.input";
                    break;
                }

                switch (priority) {
                case SDL_LOG_PRIORITY_VERBOSE:
                    log::info(where, "%s", message);
                    break;

                case SDL_LOG_PRIORITY_DEBUG:
                    log::debug(where, "%s", message);
                    break;

                case SDL_LOG_PRIORITY_INFO:
                    log::info(where, "%s", message);
                    break;

                case SDL_LOG_PRIORITY_WARN:
                    log::warning(where, "%s", message);
                    break;

                case SDL_LOG_PRIORITY_ERROR:
                    log::error(where, "%s", message);
                    break;

                case SDL_LOG_PRIORITY_CRITICAL:
                    log::error(where, "%s", message);
                    break;
                }
            }
        }

        namespace keyboard_internal {
            static uint8_t KeyboardStates[512] = { 0 };
            static uint8_t KeyboardStatesLast[512] = { 0 };
        }

        namespace mouse_internal {
            static Vector2 MouseAxis = vector2::ZERO;
            static uint32_t MouseButtonState = 0;
            static uint32_t MouseButtonStateLast = 0;

            static uint64_t left_btn_hash = 0;
            static uint64_t middle_btn_hash = 0;
            static uint64_t right_btn_hash = 0;
        }

        void init() {
            SDL_LogSetOutputFunction(&log_internal::sdl_log_output_function, nullptr);
            SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

            CE_ASSERT(SDL_Init(SDL_INIT_VIDEO) == 0);

            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "dsadasdsd");

            mouse_internal::left_btn_hash = murmur_hash_64("left", strlen("left"), 22);
            mouse_internal::middle_btn_hash = murmur_hash_64("middle", strlen("middle"), 22);
            mouse_internal::right_btn_hash = murmur_hash_64("right", strlen("right"), 22);
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

            /*Keyboard*/
            memcpy(keyboard_internal::KeyboardStates, SDL_GetKeyboardState(NULL), 512);

            /*Mouse*/
            int32_t x, y;
            mouse_internal::MouseButtonState = SDL_GetMouseState(&x, &y);
            mouse_internal::MouseAxis.x = x;
            mouse_internal::MouseAxis.y = y;
        }

        void frame_end() {
            memcpy(keyboard_internal::KeyboardStatesLast, keyboard_internal::KeyboardStates, 512);
            mouse_internal::MouseButtonStateLast = mouse_internal::MouseButtonState;
        }
    }

    namespace runtime {
        namespace window_internal {
            CE_INLINE uint32_t sdl_pos(const uint32_t pos) {
                switch (pos) {
                case window::WINDOWPOS_CENTERED:
                    return SDL_WINDOWPOS_CENTERED;

                case window::WINDOWPOS_UNDEFINED:
                    return SDL_WINDOWPOS_UNDEFINED;

                default:
                    return pos;
                }
            }

            CE_INLINE uint32_t sdl_flags(const window::WindowFlags flags) {
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
                return keyboard_internal::KeyboardStates[button_index];
            }

            bool button_pressed(const uint32_t button_index) {
                return !keyboard_internal::KeyboardStatesLast[button_index] &&
                       keyboard_internal::KeyboardStates[button_index];
            }

            bool button_released(const uint32_t button_index) {
                return keyboard_internal::KeyboardStatesLast[button_index] &&
                       !keyboard_internal::KeyboardStates[button_index];
            }
        };


        namespace mouse {
            uint32_t button_index(const char* scancode) {
                uint64_t h = murmur_hash_64(scancode, strlen(scancode), 22);

                if (h == mouse_internal::left_btn_hash) {
                    return SDL_BUTTON_LMASK;
                } else if (h == mouse_internal::middle_btn_hash) {
                    return SDL_BUTTON_MMASK;
                } else if (h == mouse_internal::right_btn_hash) {
                    return SDL_BUTTON_RMASK;
                }

                return 0;
            }

            const char* button_name(const uint32_t button_index) {
                if (button_index == SDL_BUTTON_LMASK) {
                    return "left";
                } else if (button_index == SDL_BUTTON_MMASK) {
                    return "middle";
                } else if (button_index == SDL_BUTTON_RMASK) {
                    return "right";
                }

                return "";
            }

            bool button_state(const uint32_t button_index) {
                return mouse_internal::MouseButtonState & button_index;
            }

            bool button_pressed(const uint32_t button_index) {
                return !(mouse_internal::MouseButtonStateLast & button_index) &&
                       (mouse_internal::MouseButtonState & button_index);
            }

            bool button_released(const uint32_t button_index) {
                return (mouse_internal::MouseButtonStateLast & button_index) &&
                       !(mouse_internal::MouseButtonState & button_index);
            }

            Vector2 axis() {
                return mouse_internal::MouseAxis;
            }
        };

        namespace file {
            File from_file(const char* path, const char* mode) {
                SDL_RWops* rwops = SDL_RWFromFile(path, mode);

                if (!rwops) {
                    log::warning("sys", "Open file error: %s", path, SDL_GetError());
                }

                return (struct File) {rwops};
            }

            int close(const File& f) {
                return SDL_RWclose(f.ops);
            }

            size_t read(const File& f, void* ptr, size_t size, size_t maxnum) {
                return SDL_RWread(f.ops, ptr, size, maxnum);
            }

            size_t write(const File& f, const void* ptr, size_t size, size_t num) {
                return SDL_RWwrite(f.ops, ptr, size, num);
            }

            int64_t seek(const File& f, int64_t offset, SeekWhence whence) {
                int wh = 0;

                switch (whence) {
                case SW_SEEK_SET:
                    wh = RW_SEEK_SET;
                    break;

                case SW_SEEK_CUR:
                    wh = RW_SEEK_CUR;
                    break;

                case SW_SEEK_END:
                    wh = RW_SEEK_END;
                    break;
                }

                return SDL_RWseek(f.ops, offset, wh);
            }

            int64_t tell(const File& f) {
                return SDL_RWtell(f.ops);
            }


            size_t size(const File& f) {
                size_t size;

                seek(f, 0, SW_SEEK_END);

                size = tell(f);

                seek(f, 0, SW_SEEK_SET);

                return size;
            }
        }

    }
}

