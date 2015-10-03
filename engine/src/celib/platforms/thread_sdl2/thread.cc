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

#include "celib/platforms/thread_sdl2/types.h"

#include "SDL2/SDL.h"

namespace cetech {
    namespace thread {
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

