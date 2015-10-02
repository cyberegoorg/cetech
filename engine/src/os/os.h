#pragma once

#include "common/platform/defines.h"

#include "common/container/container_types.h"
#include "common/math/math_types.h"

namespace cetech {
    namespace os {
        void init();
        void shutdown();

        void frame_start();
        void frame_end();

        uint32_t get_ticks();
    }
}