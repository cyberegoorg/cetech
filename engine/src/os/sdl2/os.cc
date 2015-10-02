#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "common/macros.h"
#include "common/log/log.h"
#include "common/asserts.h"
#include "common/container/array.inl.h"
#include "common/crypto/murmur_hash.inl.h"
#include "common/math/vector2.inl.h"
#include "application.h"
#include "os/os.h"

#include "SDL2/SDL.h"

namespace cetech {
    namespace os {
        void init() {
            CE_ASSERT(SDL_Init(SDL_INIT_EVERYTHING) == 0);
	}

        void shutdown() {
            SDL_Quit();
        }

        void frame_start() {
            SDL_Event e;

            while (SDL_PollEvent(&e) > 0) {
                switch (e.type) {
                case SDL_QUIT:
                    application_globals::app().quit();
                    break;
                }
            }
        }

        void frame_end() {
        }


        uint32_t get_ticks() {
            return SDL_GetTicks();
        }
    }
}

