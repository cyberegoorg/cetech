#pragma once

#include "celib/defines.h"

#include "celib/container/container_types.h"
#include "celib/math/math_types.h"

#if defined(CETECH_SDL2)
    #include "SDL2/SDL.h"
#endif


namespace cetech {

#if defined(CETECH_SDL2)
    typedef int (* thread_fce_t) (void* data);

    struct Thread {
        SDL_Thread* t;
    };

    struct Spinlock {
        SDL_SpinLock l;

        Spinlock() : l(0) {}
    };
#endif

    namespace thread {
        /*!
         * Create thread.
         * \param fce Thread fce.
         * \param name Thread name.
         * \param data Thread data.
         */
        Thread create_thread(thread_fce_t fce,
                             const char* name,
                             void* data);

        /*!
         * Kill thread.
         * \param thread Thread.
         */
        void kill(Thread thread);

        /*!
         * Wait.
         * \param thread Thread.
         */
        void wait(Thread thread,
                  int* status);

        /*!
         * Get id for thread.
         * \param thread Thread.
         * \return thread id.
         */
        uint32_t get_id(Thread thread);

        /*!
         * Get actual thread id.
         * \return thread id.
         */
        uint32_t id();

        /*!
         * Spin lock.
         * \param lock Spin.
         */
        void spin_lock(Spinlock& lock);

        /*!
         * Spin unlock.
         * \param lock Spin.
         */
        void spin_unlock(Spinlock& lock);
    }
}
