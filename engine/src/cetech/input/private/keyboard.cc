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

#include <unistd.h>

namespace cetech {
    namespace {
        static uint8_t KeyboardStates[512];
        static uint8_t KeyboardStatesLast[512];
    }

    namespace keyboard {
        void process_keyboard() {
            auto time = develop_manager::enter_scope("keyboard::process_keyboard");

            /*Keyboard*/
            memcpy(KeyboardStatesLast, KeyboardStates, 512);
            memcpy(KeyboardStates, SDL_GetKeyboardState(NULL), 512);

            develop_manager::leave_scope("keyboard::process_keyboard", time);
        }

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
            return !KeyboardStatesLast[button_index] &&
                   KeyboardStates[button_index];
        }

        bool button_released(const uint32_t button_index) {
            return KeyboardStatesLast[button_index] &&
                   !KeyboardStates[button_index];
        }
    };

}

#endif
