#include "application.h"

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
#include "cetech/filesystem/disk_filesystem.h"
#include "cetech/package_manager/package_resource.h"

#include "cetech/renderer/renderer.h"

#include "cetech/platform/keyboard.h"
#include "cetech/platform/mouse.h"
#include "cetech/platform/thread.h"
#include "cetech/platform/window.h"
#include "cetech/platform/dir.h"

#include "cetech/cvars/cvars.h"
#include "cetech/os/os.h"


#include "rapidjson/prettywriter.h"



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
            StringId64_t boot_pkg_name_h = stringid64::from_cstringn(cvars::boot_pkg.value_str,
                                                                    cvars::boot_pkg.str_len);

            package_manager::unload(boot_pkg_name_h);
            resource_manager::unload(resource_package::type_hash(), &boot_pkg_name_h, 1);

        }
    }

    namespace application {
        float get_delta_time() {
            return _globals.data->_delta_time;
        }
        uint32_t get_frame_id() {
            return _globals.data->_frame_id;
        }

        void init() {
            ApplictionData* data = _globals.data;

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

            data->_last_frame_ticks = os::get_ticks();
            
            init_boot();
            lua_enviroment::call_global("init");
        }

        void shutdown() {
            lua_enviroment::call_global("shutdown");

            shutdown_boot();
        }

        static void console_server_tick(void* data) {
            console_server::tick();
        }

        void run() {
            ApplictionData* data = _globals.data;

            float dt = 0.0f;
            uint32_t now_ticks = 0;
            while (data->_flags.run) {

                now_ticks = os::get_ticks();
                dt = (now_ticks - data->_last_frame_ticks) * 0.001f;
                data->_delta_time = dt;
                data->_last_frame_ticks = now_ticks;


                develop_manager::push_record_float("engine.frame_id", data->_frame_id);
                develop_manager::push_record_float("engine.delta_time", dt);
                develop_manager::push_record_float("engine.frame_rate", 1.0f / dt);

                os::frame_start();
                keyboard::frame_start();
                mouse::retrive_state();

                if (!data->_flags.daemon_mod) {
                    renderer::begin_frame();
                }

                task_manager::TaskID frame_task = task_manager::add_empty_begin(0);
                task_manager::TaskID console_server_task = task_manager::add_begin(
                    console_server_tick, nullptr, 0,
                    NULL_TASK, frame_task
                    );

                const task_manager::TaskID task_end[] = {
                    frame_task,
                    console_server_task,
                };

                task_manager::add_end(task_end, sizeof(task_end) / sizeof(task_manager::TaskID));

                //usleep(3 * 1000);
                if (!data->_flags.pause) {
                    lua_enviroment::call_global("update", "f", dt);
                    lua_enviroment::clean_temp();
                }

                //
                os::frame_end();

                keyboard::frame_end();
                mouse::swap_states();

                develop_manager::push_end_frame();
                develop_manager::send_buffer();
                develop_manager::clear();

                ++(data->_frame_id);

                task_manager::wait(frame_task);     // TODO

                if (!data->_flags.daemon_mod) {
                    renderer::end_frame();
                    window::update(data->main_window);
                }
            }

            log_globals::log().info("main", "Bye Bye");
        }

        void quit() {
            ApplictionData* data = _globals.data;
            data->_flags.run = 0;
            task_manager::stop();
        }

        bool is_run() {
            ApplictionData* data = _globals.data;
            return data->_flags.run != 0;
        }

        Platform platform() {
            return PLATFORM_LINUX;
        }
    };

    namespace application_globals {
        void init() {
            char* p = _globals.buffer;
            _globals.data = new(p) ApplictionData();
        }

        void shutdown() {
            _globals = Globals();
        }
    }
}
