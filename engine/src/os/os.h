#pragma once

#include "common/platform/defines.h"

#include "common/container/container_types.h"
#include "common/math/math_types.h"
#include "os/os_types.h"

namespace cetech {
    namespace os {
        void init();
        void shutdown();

        void frame_start();
        void frame_end();

        uint32_t get_ticks();
    }

    namespace os {
        namespace window {
            enum WindowFlags {
                WINDOW_NOFLAG = 0,
                WINDOW_FULLSCREEN = 1,

            };

            enum WindowPos {
                WINDOWPOS_CENTERED = -1,
                WINDOWPOS_UNDEFINED = -2
            };
        }

        namespace window {
            Window make_window(const char* title,
                               const int32_t x,
                               const int32_t y,
                               const int32_t width,
                               const int32_t height,
                               WindowFlags flags);

            void destroy_window(const Window& w);

            void set_title(const Window& w, const char* title);
            const char* get_title(const Window& w);
        }

        namespace keyboard {
            uint32_t button_index(const char* scancode);
            const char* button_name(const uint32_t button_index);

            bool button_state(const uint32_t button_index);
            bool button_pressed(const uint32_t button_index);
            bool button_released(const uint32_t button_index);
        };

        namespace mouse {
            uint32_t button_index(const char* scancode);
            const char* button_name(const uint32_t button_index);

            bool button_state(const uint32_t button_index);
            bool button_pressed(const uint32_t button_index);
            bool button_released(const uint32_t button_index);

            Vector2 axis();
        };

        namespace file {
            enum SeekWhence {
                SW_SEEK_SET = 1,
                SW_SEEK_CUR,
                SW_SEEK_END
            };

            File from_file(const char* path, const char* mode);

            bool is_null(const File& f);

            int close(const File& f);
            size_t read(const File& f, void* ptr, size_t size, size_t maxnum);
            size_t write(const File& f, const void* ptr, size_t size, size_t num);

            int64_t seek(const File& f, int64_t offset, SeekWhence whence);
            int64_t tell(const File& f);

            size_t size(const File& f);
        }

        namespace dir {
            bool mkdir(const char* path);
            bool mkpath(const char* path);

            void listdir(const char* name, Array < char* >& files);
            void listdir_free(Array < char* >& files);
        }

        namespace cpu {
            /*!
             * Get cpu core count.
             * \return Core count.
             */
            uint32_t core_count();
        }

        namespace thread {
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
}