#pragma once

#include "celib/defines.h"

namespace cetech {
    namespace keyboard {
        void process_keyboard();

        uint32_t button_index(const char* scancode);
        const char* button_name(const uint32_t button_index);

        bool button_state(const uint32_t button_index);
        bool button_pressed(const uint32_t button_index);
        bool button_released(const uint32_t button_index);
    }
}