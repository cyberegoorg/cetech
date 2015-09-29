#pragma once

#include "SDL2/SDL.h"

namespace cetech {
    namespace os {
        struct Window {
            SDL_Window* wnd;
        };

        struct File {
            SDL_RWops* ops;
        };

        typedef int (* thread_fce_t) (void* data);

        struct Thread {
            SDL_Thread* t;
        };

        struct Spinlock {
            SDL_SpinLock l;
            
            Spinlock(): l(0) {
            }
        };
    }
}