#pragma once

namespace cetech1 {
    namespace platform {
        inline void init();
        inline void shutdown();
    }
}

#ifdef CETECH_PLATFORM_SDL2
    #include "sdl2/platform.h"
#endif