#pragma once

#include <cinttypes>

#include "celib/string/stringid_types.h"
#include "celib/memory/memory_types.h"
#include "celib/platform/types.h"

namespace cetech {
    struct Renderer {
        struct Implementation;

        Renderer(Allocator& allocator);
        ~Renderer();

        void init(Window window);

        void begin_frame();
        void end_frame();
        void resize(uint32_t w, uint32_t h);

        static Renderer* make(Allocator& allocator);
        static void destroy(Allocator& allocator, Renderer* pm);

        Allocator& _allocator;
        Implementation* _impl;
    };
}