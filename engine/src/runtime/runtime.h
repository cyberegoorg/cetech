#pragma once

#include "common/math/math_types.h"
#include "runtime_types.h"

namespace cetech1 {
    namespace runtime {
        void init();
        void shutdown();

        void frame_start();
        void frame_end();
    }

    namespace runtime {
        namespace window {
            enum WindowFlags {
                WINDOW_NOFLAG = 0,
                WINDOW_FULLSCREEN = 1,
            };

            enum WindowPos {
                WINDOWPOS_CENTERED = -1,
                WINDOWPOS_UNDEFINED = -2
            };
        }

        namespace window {
            Window make_window(const char* title,
                               const int32_t x,
                               const int32_t y,
                               const int32_t width,
                               const int32_t height,
                               WindowFlags flags);

            void destroy_window(const Window& w);

            void set_title(const Window& w, const char* title);
            const char* get_title(const Window& w);
        }

        namespace keyboard {
            uint32_t button_index(const char* scancode);
            const char* button_name(const uint32_t button_index);

            bool button_state(const uint32_t button_index);
            bool button_pressed(const uint32_t button_index);
            bool button_released(const uint32_t button_index);
        };
	
        namespace mouse {
            uint32_t button_index(const char* scancode);
            const char* button_name(const uint32_t button_index);

            bool button_state(const uint32_t button_index);
            bool button_pressed(const uint32_t button_index);
            bool button_released(const uint32_t button_index);

	    Vector2 axis();
        };
    }
}

#ifdef CETECH_RUNTIME_SDL2
    #include "runtime/sdl2/runtime.h"
#endif
