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
#include "application/application.h"

#include "platforms/thread_sdl2/types.h"

#include "SDL2/SDL.h"

namespace cetech {
    namespace thread {
        void init() {};
        void shutdown() {};

        void frame_start() {};
        void frame_end() {};

        Thread create_thread(thread_fce_t fce, const char* name, void* data) {
            return (Thread) {
                       SDL_CreateThread(fce, name, data)
            };
        }

        void kill(Thread thread) {
            SDL_DetachThread(thread.t);
        }

        uint32_t get_id(Thread thread) {
            return SDL_GetThreadID(thread.t);
        }

        uint32_t id() {
            return SDL_ThreadID();
        }

        void spin_lock(Spinlock& lock) {
            SDL_AtomicLock(&(lock.l));
        }

        void spin_unlock(Spinlock& lock) {
            SDL_AtomicUnlock(&(lock.l));
        }
    }
}

