#include "cetech/application/application.h"

#include "celib/macros.h"
#include "celib/memory/memory.h"
#include "celib/command_line/command_line.h"
#include "cetech/log_system/handlers.h"
#include "celib/container/array.inl.h"
#include "celib/string/stringid.inl.h"

#include "cetech/lua/lua_enviroment.h"

#include "cetech/resource_manager/resource_manager.h"
#include "cetech/resource_compiler/resource_compiler.h"
#include "cetech/package_manager/package_manager.h"
#include "cetech/develop/develop_manager.h"
#include "cetech/package_manager/package_resource.h"

#include "cetech/renderer/renderer.h"

#include "cetech/input/keyboard.h"
#include "cetech/input/mouse.h"
#include "cetech/thread/thread.h"

#include "cetech/cvars/cvars.h"


#include "rapidjson/prettywriter.h"

#include <unistd.h>

namespace cetech {
    namespace {
        using namespace application;

        struct ApplictionData {
            Window main_window;
            uint32_t _frame_id;
            uint32_t _last_frame_ticks;

            float _delta_time;

            struct {
                char run : 1;
                char pause : 1;
                char daemon_mod : 1;
            } _flags;

            ApplictionData() : _frame_id(0), _last_frame_ticks(0), _delta_time(0) {
                _flags = {0, 0, 0};
            }
        };

        struct Globals {
            static const int MEMORY = sizeof(ApplictionData);
            char buffer[MEMORY];

            ApplictionData* data;

            Globals() : data(0) {}
        } _globals;

        void init_boot() {
            package_manager::load_boot_package();

            // Execute boot script
            StringId64_t boot_script_name_h = stringid64::from_cstringn(cvars::boot_script.value_str,
                                                                        cvars::boot_script.str_len);

            const resource_lua::Resource* res_lua;
            res_lua = (const resource_lua::Resource*) resource_manager::get(
                resource_lua::type_hash(), boot_script_name_h);
            lua_enviroment::execute_resource(res_lua);
        }

        void shutdown_boot() {
            package_manager::unload_boot_package();

            StringId64_t boot_pkg_name_h = stringid64::from_cstringn(cvars::boot_pkg.value_str,
                                                                     cvars::boot_pkg.str_len);

            package_manager::unload(boot_pkg_name_h);
            resource_manager::unload(resource_package::type_hash(), &boot_pkg_name_h, 1);

        }


        inline void init_os() {
#if defined(CETECH_SDL2)
            CE_ASSERT_MSG(SDL_Init(SDL_INIT_EVERYTHING) == 0, SDL_GetError());
#endif
        }

        inline void shutdown_os() {
#if defined(CETECH_SDL2)
            SDL_Quit();
#endif
        }

        inline void process_os() {
#if defined(CETECH_SDL2)
            SDL_Event e;
            while (SDL_PollEvent(&e) > 0) {
                switch (e.type) {
                case SDL_QUIT:
                    application::quit();
                    break;
                }
            }

#endif
        }

    }

    namespace application {
        uint32_t get_ticks() {
#if defined(CETECH_SDL2)
            return SDL_GetTicks();
#endif
        }

        float get_delta_time() {
            return _globals.data->_delta_time;
        }
        uint32_t get_frame_id() {
            if (!_globals.data) {
                return 0;
            }

            return _globals.data->_frame_id;
        }

        void init() {
            ApplictionData* data = _globals.data;

            init_os();

            if (command_line_globals::has_argument("daemon", 'd')) {
                data->_flags.daemon_mod = 1;
            }

            data->_flags.run = 1;

            if (!data->_flags.daemon_mod) {
                if (command_line_globals::has_argument("wid")) {
                    char* ptr;
                    long wid;

                    wid = strtol(command_line_globals::get_parameter("wid"), &ptr, 10);
                    data->main_window = window::make_from((void*)wid);

                } else {
                    data->main_window = window::make_window(
                        "cetech runtime",
                        window::WINDOWPOS_CENTERED, window::WINDOWPOS_CENTERED,
                        cvars::screen_width.value_i, cvars::screen_height.value_i,
                        window::WINDOW_NOFLAG
                        );
                }

                renderer::init(data->main_window, renderer::RenderType::OpenGL);
            }

            data->_last_frame_ticks = get_ticks();

            init_boot();
            lua_enviroment::call_global("init");
        }

        void shutdown() {
            log::info("main", "Bye Bye");

            lua_enviroment::call_global("shutdown");

            shutdown_boot();
            renderer_globals::shutdown();
            shutdown_os();
        }

        static void process_mouse(void* data) {
            mouse::process_mouse();
            usleep(2 * 1000);
        }

        static void process_keyboard(void* data) {
            keyboard::process_keyboard();
            usleep(2 * 1000);
        }

        static void console_server_tick(void* data) {
            console_server::tick();
        }

        static void frame_tick(void* _data) {
            CE_CHECK_PTR(_globals.data);
            ApplictionData& data = *_globals.data;

            float dt = 0.0f;
            uint32_t now_ticks = 0;

            now_ticks = get_ticks();

            dt = (now_ticks - data._last_frame_ticks) * 0.001f;
            data._delta_time = dt;
            data._last_frame_ticks = now_ticks;


    #if defined(CETECH_DEVELOP)
            develop_manager::push_record_float("engine.frame_id", data._frame_id);
            develop_manager::push_record_float("engine.delta_time", dt);
            develop_manager::push_record_float("engine.frame_rate", 1.0f / dt);
    #endif

            if (!data._flags.daemon_mod) {
                renderer::begin_frame();
            }

            if (!data._flags.pause) {
                lua_enviroment::call_global("update", "f", dt);
                lua_enviroment::clean_temp();
            }

#if defined(CETECH_DEVELOP)
#endif

            if (!data._flags.daemon_mod) {
                renderer::end_frame();
                window::update(data.main_window);
            }

            usleep(2 * 1000);
        }

        void run() {
            init();

            CE_CHECK_PTR(_globals.data);
            ApplictionData& data = *_globals.data;

            develop_manager::send_buffer();
            console_server::tick();
            
            while (data._flags.run) {
                develop_manager::clear();
                ++data._frame_id;

                auto time = develop_manager::enter_scope("Application::Update()");

                console_server::tick();
                develop_manager::push_begin_frame();

                process_os();
                keyboard::process_keyboard();
                mouse::process_mouse();

                float dt = 0.0f;
                uint32_t now_ticks = 0;

                now_ticks = get_ticks();

                dt = (now_ticks - data._last_frame_ticks) * 0.001f;
                data._delta_time = dt;
                data._last_frame_ticks = now_ticks;


                develop_manager::push_record_float("engine.frame_id", data._frame_id);
                develop_manager::push_record_float("engine.delta_time", dt);
                develop_manager::push_record_float("engine.frame_rate", 1.0f / dt);

                if (!data._flags.daemon_mod) {
                    renderer::begin_frame();
                }


                if (!data._flags.pause) {
                    auto time = develop_manager::enter_scope("Game::Update()");
                    lua_enviroment::call_global("update", "f", dt);
                    lua_enviroment::clean_temp();
                    usleep(10 * 100);
                    develop_manager::leave_scope("Game::Update()", time);
                }

                if (!data._flags.daemon_mod) {
                    renderer::end_frame();
                    window::update(data.main_window);
                }

                develop_manager::push_end_frame();
                develop_manager::leave_scope("Application::Update()", time);
                develop_manager::flush_all_stream_buffer();
                develop_manager::send_buffer();

                //                 if (task_manager::open_task_count() != 0) {
                //                     printf("c: %d \n", task_manager::open_task_count());
                //                     CE_ASSERT(task_manager::open_task_count() == 0);
                //                 }

            }

            shutdown();
        }

        void quit() {
            ApplictionData* data = _globals.data;
            data->_flags.run = 0;
            //task_manager::stop();
        }

        bool is_run() {
            ApplictionData* data = _globals.data;
            if (!data) {
                return false;
            }

            return data->_flags.run != 0;
        }

        Platform platform() {
            return PLATFORM_LINUX;
        }
    };

    namespace application_globals {
        void init() {
            log::info("appplication_globals", "Init");

            char* p = _globals.buffer;
            _globals.data = new(p) ApplictionData();
        }

        void shutdown() {
            log::info("appplication_globals", "Shutdown");

            _globals.data->~ApplictionData();
            _globals = Globals();

        }
    }
}
