#pragma once

#include <cinttypes>

#include "celib/string/stringid_types.h"
#include "celib/memory/memory_types.h"
#include "cetech/platform/types.h"

namespace cetech {
    namespace renderer {
        struct RenderType {
            enum Enum {
                Null = 0,

                Direct3D9,
                Direct3D11,
                Direct3D12,
                Metal,
                OpenGLES,
                OpenGL,
                Vulkan,

                Count
            };
        };

        void init(Window window, RenderType::Enum render_type);
        void shutdown();

        void begin_frame();
        void end_frame();
        void resize(uint32_t w, uint32_t h);
    }

    namespace renderer_globals {
        void init();
        void shutdown();
    }
}