#ifdef CETECH_RUNTIME_SDL2

#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "SDL2/SDL.h"

namespace cetech {
    namespace cpu {
        uint32_t core_count() {
            return SDL_GetCPUCount();
        }
    }
}

#endif