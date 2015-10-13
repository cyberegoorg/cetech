#pragma once

#include <cinttypes>

#include "celib/string/stringid_types.h"
#include "celib/memory/memory_types.h"
#include <celib/platform/types.h>

namespace cetech {
    class Renderer {
        public:
            virtual ~Renderer() {}

            virtual void init(Window window) = 0;

            virtual void begin_frame() = 0;
            virtual void end_frame() = 0;
	    virtual void resize(uint32_t w, uint32_t h) = 0;

            static Renderer* make(Allocator& allocator);
            static void destroy(Allocator& allocator, Renderer* pm);
    };
}