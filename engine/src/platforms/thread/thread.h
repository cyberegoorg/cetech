#pragma once

#include "common/platform/defines.h"

#include "common/container/container_types.h"
#include "common/math/math_types.h"
#include "platforms/thread/thread_types.h"

namespace cetech {

    namespace thread {
        void init();
        void shutdown();

        void frame_start();
        void frame_end();


        /*!
         * Create thread.
         * \param fce Thread fce.
         * \param name Thread name.
         * \param data Thread data.
         */
        Thread create_thread(thread_fce_t fce, const char* name, void* data);

        /*!
         * Kill thread.
         * \param thread Thread.
         */
        void kill(Thread thread);

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