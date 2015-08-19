#pragma once

#include "SDL2/SDL.h"

#include "common/asserts.h"

namespace cetech1 {
    namespace runtime {
        void init() {
            CE_ASSERT(SDL_Init(SDL_INIT_VIDEO) < 0);
        }
        
        void shutdown() {
        }
    }
}
