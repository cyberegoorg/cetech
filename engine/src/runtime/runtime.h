#pragma once

namespace cetech1 {
    namespace runtime {
        void init();
        void shutdown();
    }
}

#ifdef CETECH_RUNTIME_SDL2
    #include "runtime/sdl2/runtime.h"
#endif
