#pragma once

#include "celib/defines.h"

#include "celib/math/math_types.h"

namespace cetech {
    namespace mouse {
        void init();
        void frame_start();
        void frame_end();
        //void shutdown();

        uint32_t button_index(const char* scancode);
        const char* button_name(const uint32_t button_index);

        bool button_state(const uint32_t button_index);
        bool button_pressed(const uint32_t button_index);
        bool button_released(const uint32_t button_index);

        Vector2 axis();
    };
}