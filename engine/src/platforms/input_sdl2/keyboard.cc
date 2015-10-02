#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "common/macros.h"
#include "common/log/log.h"
#include "common/asserts.h"
#include "common/container/array.inl.h"
#include "common/crypto/murmur_hash.inl.h"
#include "common/math/vector2.inl.h"
#include "application/application.h"
#include "os/os.h"

#include "SDL2/SDL.h"

namespace cetech {
    namespace keyboard_internal {
        static uint8_t KeyboardStates[512];
        static uint8_t KeyboardStatesLast[512];
    }

    namespace keyboard {

        void init() {}
        void shutdown() {}

        void frame_start() {
            /*Keyboard*/
            memcpy(keyboard_internal::KeyboardStates, SDL_GetKeyboardState(NULL), 512);
        }

        void frame_end() {
            memcpy(keyboard_internal::KeyboardStatesLast, keyboard_internal::KeyboardStates, 512);
        }


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

}

