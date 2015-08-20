#pragma once

#include "SDL2/SDL.h"

#include "common/asserts.h"
#include "common/murmur_hash.h"
#include "common/math/vector2.h"
#include "runtime/runtime.h"

namespace cetech1 {
    namespace runtime {
        static uint8_t KeyboardStates[512] = { 0 };
        static uint8_t KeyboardStatesLast[512] = { 0 };


	static Vector2 MouseAxis = vector2::ZERO;
	static uint32_t MouseButtonState = 0;
	static uint32_t MouseButtonStateLast = 0;
	
	static uint64_t left_btn_hash = 0;
	static uint64_t middle_btn_hash = 0;
	static uint64_t right_btn_hash = 0;
	
        void init() {
            CE_ASSERT(SDL_Init(SDL_INIT_VIDEO) == 0);
	    
	    left_btn_hash = murmur_hash_64("left", strlen("left"), 22);
	    middle_btn_hash = murmur_hash_64("middle", strlen("middle"), 22);
	    right_btn_hash = murmur_hash_64("right", strlen("right"), 22);
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
            memcpy(KeyboardStates, SDL_GetKeyboardState(NULL), 512);

	    /*Mouse*/
	    int32_t x, y;
	    MouseButtonState = SDL_GetMouseState(&x, &y);
	    MouseAxis.x = x;
	    MouseAxis.y = y;
        }

        void frame_end() {
            memcpy(KeyboardStatesLast, KeyboardStates, 512);
	    MouseButtonStateLast = MouseButtonState;
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
	

        namespace mouse {
            uint32_t button_index(const char* scancode) {
		uint64_t h = murmur_hash_64(scancode, strlen(scancode), 22);
		
		if (h == left_btn_hash)
		    return SDL_BUTTON_LMASK;
		else if (h == middle_btn_hash)
		    return SDL_BUTTON_MMASK;
		else if (h == right_btn_hash)
		    return SDL_BUTTON_RMASK;
		
		return 0;
	    }

            const char* button_name(const uint32_t button_index) {
		if (button_index == SDL_BUTTON_LMASK)
			return "left";
		else if (button_index == SDL_BUTTON_MMASK)
			return "middle";
		else if (button_index == SDL_BUTTON_RMASK)
			return "right";

		return "";
	    }

            bool button_state(const uint32_t button_index) {
		return MouseButtonState & button_index;
	    }
	    
            bool button_pressed(const uint32_t button_index) {
		return !(MouseButtonStateLast & button_index) && (MouseButtonState & button_index);
	    }
	    
            bool button_released(const uint32_t button_index){
		return (MouseButtonStateLast & button_index) && !(MouseButtonState & button_index);
	    }

	    Vector2 axis() {
		return MouseAxis;
	    }
        };
	
    }
}
