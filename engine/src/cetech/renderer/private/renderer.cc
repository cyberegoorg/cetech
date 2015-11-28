#include "cetech/renderer/renderer.h"
#include "cetech/cvars/cvars.h"

#include "celib/macros.h"
#include "celib/memory/memory.h"
#include "celib/math/math_types.h"
#include "celib/math/matrix44.inl.h"


#include "bx/uint32_t.h"
#include "bgfx/bgfx.h"
#include "bgfx/bgfxplatform.h"
#include "bgfx/bgfxdefines.h"

#include "cetech/application/application.h"
#include "cetech/renderer/texture_resource.h"
#include "celib/string/stringid.inl.h"

#if defined(CETECH_DEVELOP)
    #include "cetech/develop/console_server.h"
#endif

/********
* SDL2 *
********/
#if defined(CETECH_SDL2)
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
    bgfx::osxSetNSWindow(wmi.info.cocoa.window);
#elif BX_PLATFORM_WINDOWS
    bgfx::winSetHwnd(wmi.info.win.window);
#endif  /* BX_PLATFORM_ */

    return 1;
}
#endif
/*********/


namespace cetech {
    namespace {
        using namespace renderer;
        struct RendererData {
            uint32_t frame_id;
            uint32_t resize_w;
            uint32_t resize_h;
            bool need_resize;

            RendererData() : frame_id(0), resize_w(0), resize_h(0), need_resize(1) {}
        };

        struct Globals {
            static const int MEMORY = sizeof(RendererData);
            char buffer[MEMORY];

            RendererData* data;

            Globals() : data(0) {}
        } _globals;

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

        static void cmd_renderer_resize(const rapidjson::Document& in,
                                        rapidjson::Document& out) {
            CE_UNUSED(out);
            const uint32_t width = in["args"]["width"].GetInt();
            const uint32_t height = in["args"]["height"].GetInt();
            renderer::resize(width, height);
        }
    }

    namespace renderer {
        void init(Window window,
                  RenderType::Enum render_type) {
#if defined(CETECH_DEVELOP)
            console_server::register_command("renderer.resize", cmd_renderer_resize);
#endif

            #if defined(CETECH_SDL2)
            sdlSetWindow(window.wnd);
            #endif

            bgfx::init(_bgfx_render_type(render_type));
            resize(cvars::screen_width.value_i, cvars::screen_height.value_i);
        };

        void resize(uint32_t w,
                    uint32_t h) {
            RendererData* data = _globals.data;

            data->need_resize = true;
            data->resize_w = w;
            data->resize_h = h;
        }

        void begin_frame() {
            RendererData* data = _globals.data;


            if (data->need_resize) {
                cvar::set(cvars::screen_width, (int)data->resize_w);
                cvar::set(cvars::screen_height, (int)data->resize_h);

                bgfx::reset(data->resize_w, data->resize_h, 0);
                bgfx::setViewRect(0, 0, 0, data->resize_w, data->resize_h);
                data->need_resize = false;
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
            RendererData* data = _globals.data;
            data->frame_id = bgfx::frame();
        }
    }

    namespace renderer_globals {
        void init() {
            log::info("renderer_globals", "Init");

            char* p = _globals.buffer;
            _globals.data = new(p) RendererData();
        }

        void shutdown() {
            log::info("renderer_globals", "Shutdown");

            bgfx::shutdown();
            _globals.data->~RendererData();
            _globals = Globals();
        }
    }
}
