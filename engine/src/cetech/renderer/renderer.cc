#include "cetech/renderer/renderer.h"
#include "cetech/cvars/cvars.h"

#include "celib/memory/memory.h"

#include "bgfx/bgfx.h"
#include "bgfx/bgfxplatform.h"
#include "bgfx/bgfxdefines.h"

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
        public:
            friend class Renderer;

            uint32_t frame_id;
            bool need_resize;
            uint32_t resize_w, resize_h;

            Implementation() : frame_id(0) {
                need_resize = false;
                resize_h = 0;
                resize_w = 0;
            }

            ~Implementation()  {
                bgfx::shutdown();
            }

            void init(Window window)  {
                sdlSetWindow(window.wnd);

                bgfx::init(bgfx::RendererType::OpenGL);
                resize(cvars::screen_width.value_i, cvars::screen_height.value_i);
            };

            void resize(uint32_t w, uint32_t h)  {
                need_resize = true;
                resize_w = w;
                resize_h = h;
            }

            void begin_frame()  {
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

                bgfx::dbgTextClear(0, 0);
                bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: Initialization and debug text.");

                bgfx::submit(0, BGFX_INVALID_HANDLE);
            }

            void end_frame()  {
                frame_id = bgfx::frame();
            }
    };

    Renderer::Renderer(Allocator& allocator): _allocator(allocator), _impl(MAKE_NEW(_allocator, Implementation)) {
    }

    Renderer::~Renderer() {
        MAKE_DELETE(_allocator, Implementation, _impl);
    }

    void Renderer::init(Window window) {
        _impl->init(window);
    }

    void Renderer::begin_frame(){
        _impl->begin_frame();
    }
    
    void Renderer::end_frame(){
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
