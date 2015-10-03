#pragma once

#include "celib/defines.h"

#include "celib/container/container_types.h"
#include "celib/math/math_types.h"

namespace cetech {
    namespace os {
        void init();
        void shutdown();

        void frame_start();
        void frame_end();

        uint32_t get_ticks();
    }
}