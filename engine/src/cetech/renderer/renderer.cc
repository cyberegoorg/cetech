#include "cetech/renderer/renderer.h"
#include "cetech/cvars/cvars.h"

#include "celib/memory/memory.h"

#include "bgfx/bgfx.h"
#include "bgfx/bgfxplatform.h"
#include "bgfx/bgfxdefines.h"

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
    class RendererImplementation : public Renderer {
        public:
            friend class Renderer;

            uint32_t frame_id;

            RendererImplementation() : frame_id(0) {}

            virtual ~RendererImplementation() final {
                bgfx::shutdown();
            }

            virtual void init(Window window) final {
                sdlSetWindow(window.wnd);

                bgfx::init(bgfx::RendererType::OpenGL, NULL, NULL);
                bgfx::reset(cvars::screen_width.value_i, cvars::screen_height.value_i, 0);
		bgfx::setViewClear(
                    0
                    , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
                    , 0x66CCFFff
                    , 1.0f
                    , 0);
            };


	    virtual void resize(uint32_t w, uint32_t h) final {
	      bgfx::reset(w, h, 0);
	    }

            virtual void begin_frame() final {
                bgfx::submit(0, BGFX_INVALID_HANDLE);

                bgfx::setDebug(BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT);
                bgfx::setViewRect(0, 0, 0, cvars::screen_width.value_i, cvars::screen_height.value_i);
                bgfx::dbgTextClear(0, 0);
                bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: Initialization and debug text.");

                bgfx::submit(0, BGFX_INVALID_HANDLE);
            }

            virtual void end_frame() final {
                frame_id = bgfx::frame();
            }
    };

    Renderer* Renderer::make(Allocator& allocator) {
        return MAKE_NEW(allocator, RendererImplementation);
    }

    void Renderer::destroy(Allocator& allocator, Renderer* pm) {
        MAKE_DELETE(memory_globals::default_allocator(), Renderer, pm);
    }
}
