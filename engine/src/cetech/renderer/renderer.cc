#include "cetech/renderer/renderer.h"
#include "cetech/cvars/cvars.h"

#include "celib/memory/memory.h"
#include "celib/math/math_types.h"
#include "celib/math/matrix44.inl.h"


#include "bx/uint32_t.h"
#include "bgfx/bgfx.h"
#include "bgfx/bgfxplatform.h"
#include "bgfx/bgfxdefines.h"

#include "cetech/application/application.h"
#include "cetech/renderer/texture/texture_resource.h"
#include "celib/string/stringid.inl.h"

// TODO: rewrite,
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
uint8_t sdlSetWindow(SDL_Window* _window) {
    SDL_SysWMinfo wmi;

    SDL_VERSION(&wmi.version);

    if (!SDL_GetWindowWMInfo(_window, &wmi)) {
        return 0;
    }

#if BX_PLATFORM_LINUX || BX_PLATFORM_FREEBSD
    bgfx::x11SetDisplayWindow(wmi.info.x11.display, wmi.info.x11.window);
#elif BX_PLATFORM_OSX
    osxSetNSWindow(wmi.info.cocoa.window);
#elif BX_PLATFORM_WINDOWS
    winSetHwnd(wmi.info.win.window);
#endif  /* BX_PLATFORM_ */

    return 1;
}

namespace cetech {
    struct Renderer::Implementation {
        friend class Renderer;

        uint32_t frame_id;
        uint32_t resize_w;
        uint32_t resize_h;
        bool need_resize;

        CE_INLINE bgfx::RendererType::Enum _bgfx_render_type(RenderType::Enum render_type) {
            switch (render_type) {
            case RenderType::Direct3D9:
                return bgfx::RendererType::Direct3D9;

            case RenderType::Direct3D11:
                return bgfx::RendererType::Direct3D11;

            case RenderType::Direct3D12:
                return bgfx::RendererType::Direct3D12;

            case RenderType::Metal:
                return bgfx::RendererType::Metal;

            case RenderType::OpenGLES:
                return bgfx::RendererType::OpenGLES;

            case RenderType::OpenGL:
                return bgfx::RendererType::OpenGL;

            case RenderType::Vulkan:
                return bgfx::RendererType::Vulkan;

            default:
                return bgfx::RendererType::Null;
            }
        }

        void init(Window window, RenderType::Enum render_type) {
            memset(this, 0, sizeof(Renderer::Implementation));

            sdlSetWindow(window.wnd);

            bgfx::init(_bgfx_render_type(render_type));
            resize(cvars::screen_width.value_i, cvars::screen_height.value_i);
        };


        void shutdown() {
            bgfx::shutdown();
        }

        void resize(uint32_t w, uint32_t h) {
            need_resize = true;
            resize_w = w;
            resize_h = h;
        }

        void begin_frame() {
            if (need_resize) {
                cvar::set(cvars::screen_width, (int)resize_w);
                cvar::set(cvars::screen_height, (int)resize_h);

                bgfx::reset(resize_w, resize_h, 0);
                bgfx::setViewRect(0, 0, 0, resize_w, resize_h);
                need_resize = false;
            }

            bgfx::setDebug(BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT);
            bgfx::setViewClear(
                0
                , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
                , 0x66CCFFff
                , 1.0f
                , 0);

            bgfx::touch(0);

            bgfx::dbgTextClear();

            bgfx::submit(0, BGFX_INVALID_HANDLE);
        }

        void end_frame() {
            frame_id = bgfx::frame();
        }
    };

    Renderer::Renderer(Allocator& allocator) : _allocator(allocator), _impl(MAKE_NEW(_allocator, Implementation)) {}

    Renderer::~Renderer() {
        MAKE_DELETE(_allocator, Implementation, _impl);
    }

    void Renderer::init(Window window, RenderType::Enum render_type) {
        _impl->init(window, render_type);
    }

    void Renderer::shutdown() {
        _impl->shutdown();
    }


    void Renderer::begin_frame() {
        _impl->begin_frame();
    }

    void Renderer::end_frame() {
        _impl->end_frame();
    }

    void Renderer::resize(uint32_t w, uint32_t h) {
        _impl->resize(w, h);
    }

    Renderer* Renderer::make(Allocator& allocator) {
        return MAKE_NEW(allocator, Renderer, allocator);
    }

    void Renderer::destroy(Allocator& allocator, Renderer* pm) {
        MAKE_DELETE(allocator, Renderer, pm);
    }
}
