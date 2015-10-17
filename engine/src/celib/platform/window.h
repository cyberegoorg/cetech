#pragma once

#include "celib/defines.h"

#include "celib/container/container_types.h"
#include "celib/math/math_types.h"
#include "celib/platform/types.h"

namespace cetech {
    namespace window {
        enum WindowFlags {
            WINDOW_NOFLAG = 0,
            WINDOW_FULLSCREEN = 1,

        };

        enum WindowPos {
            WINDOWPOS_CENTERED = 1,
            WINDOWPOS_UNDEFINED = 2
        };

        Window make_window(const char* title,
                           const int32_t x,
                           const int32_t y,
                           const int32_t width,
                           const int32_t height,
                           WindowFlags flags);

        Window make_from(void* hndl);

        void destroy_window(const Window& w);

        void set_title(const Window& w, const char* title);
        const char* get_title(const Window& w);

        void update(const Window& w);

        void resize(const Window& w, uint32_t width, uint32_t height);
    }
}