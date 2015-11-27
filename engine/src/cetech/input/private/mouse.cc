#ifdef CETECH_SDL2

#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "celib/macros.h"
#include "cetech/log/log.h"
#include "celib/asserts.h"
#include "celib/container/array.inl.h"
#include "celib/crypto/murmur_hash.inl.h"
#include "celib/math/vector2.inl.h"
#include "cetech/application/application.h"

#include "SDL2/SDL.h"

#include "unistd.h"

namespace cetech {
    namespace mouse_internal {
        static Vector2 MouseAxis;
        static uint32_t MouseButtonState;
        static uint32_t MouseButtonStateLast;

        static uint64_t left_btn_hash;
        static uint64_t middle_btn_hash;
        static uint64_t right_btn_hash;
    }

    namespace mouse {
        void init() {
            mouse_internal::left_btn_hash = murmur_hash_64("left", strlen("left"), 22);
            mouse_internal::middle_btn_hash = murmur_hash_64("middle", strlen("middle"), 22);
            mouse_internal::right_btn_hash = murmur_hash_64("right", strlen("right"), 22);
        }

        void process_mouse() {
            auto time = develop_manager::enter_scope("keyboard::process_keyboard");

            mouse_internal::MouseButtonStateLast = mouse_internal::MouseButtonState;

            /*Mouse*/
            int32_t x, y;
            x = y = 0;

            mouse_internal::MouseButtonState = SDL_GetMouseState(&x, &y);

            mouse_internal::MouseAxis.x = x;
            mouse_internal::MouseAxis.y = y;

            develop_manager::leave_scope("keyboard::process_mouse", time);
        }

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

}

#endif
