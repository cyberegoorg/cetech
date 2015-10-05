#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "celib/macros.h"
#include "celib/log/log.h"
#include "celib/asserts.h"
#include "celib/container/array.inl.h"
#include "celib/crypto/murmur_hash.inl.h"
#include "celib/math/vector2.inl.h"
#include "cetech/application/application.h"
#include "cetech/os/os.h"

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

        void frame_end() {}


        uint32_t get_ticks() {
            return SDL_GetTicks();
        }
    }
}
