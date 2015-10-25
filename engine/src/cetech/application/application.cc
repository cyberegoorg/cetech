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
    class ApplicationImplementation : public Application {
        public:
            friend class Application;

            uint32_t _frame_id;
            uint32_t _last_frame_ticks;

            float _delta_time;

            struct {
                char run : 1;
                char pause : 1;
                char daemon_mod : 1;
            } _flags;

            ApplicationImplementation() : _frame_id(0), _last_frame_ticks(0), _delta_time(0) {
                _flags = {0, 0, 0};

            }

            virtual float get_delta_time() const final {
                return this->_delta_time;
            }
            virtual uint32_t get_frame_id() const final {
                return this->_frame_id;
            }


            Window main_window;
            virtual void init() final {
                if (command_line_globals::has_argument("daemon", 'd')) {
                    _flags.daemon_mod = 1;
                }

                _flags.run = 1;

                if (!_flags.daemon_mod) {
                    if (command_line_globals::has_argument("wid")) {
                        char* ptr;
                        long wid;

                        wid = strtol(command_line_globals::get_parameter("wid"), &ptr, 10);
                        main_window = window::make_from((void*)wid);

                    } else {
                        main_window = window::make_window(
                            "cetech runtime",
                            window::WINDOWPOS_CENTERED, window::WINDOWPOS_CENTERED,
                            cvars::screen_width.value_i, cvars::screen_height.value_i,
                            window::WINDOW_NOFLAG
                            );
                    }

                    renderer::init(main_window, renderer::RenderType::OpenGL);
                }

                this->_last_frame_ticks = os::get_ticks();

                lua_enviroment::call_global("init");
            }

            virtual void shutdown() final {
                lua_enviroment::call_global("shutdown");

            }

            static void console_server_tick(void* data) {
                console_server::tick();
            }

            virtual void run() final {
                float dt = 0.0f;
                uint32_t now_ticks = 0;
                while (_flags.run) {

                    now_ticks = os::get_ticks();
                    dt = (now_ticks - this->_last_frame_ticks) * 0.001f;
                    this->_delta_time = dt;
                    this->_last_frame_ticks = now_ticks;


                    develop_manager::push_record_float("engine.frame_id", _frame_id);
                    develop_manager::push_record_float("engine.delta_time", dt);
                    develop_manager::push_record_float("engine.frame_rate", 1.0f / dt);

                    os::frame_start();
                    keyboard::frame_start();
                    mouse::retrive_state();

                    if (!_flags.daemon_mod) {
                        renderer::begin_frame();
                    }

                    task_manager::TaskID frame_task = task_manager::add_empty_begin(0);
                    task_manager::TaskID console_server_task = task_manager::add_begin(
                        console_server_tick, this, 0,
                        NULL_TASK, frame_task
                        );

                    const task_manager::TaskID task_end[] = {
                        frame_task,
                        console_server_task,
                    };

                    task_manager::add_end(task_end, sizeof(task_end) / sizeof(task_manager::TaskID));

                    //usleep(3 * 1000);
                    if (!_flags.pause) {
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

                    ++(this->_frame_id);

                    task_manager::wait(frame_task); // TODO

                    if (!_flags.daemon_mod) {
                        renderer::end_frame();
                        window::update(main_window);
                    }
                }

                log_globals::log().info("main", "Bye Bye");
            }

            virtual void quit() final {
                _flags.run = 0;
                task_manager::stop();
            }

            virtual bool is_run() final {
                return _flags.run != 0;
            }

            virtual Platform platform() final {
                return PLATFORM_LINUX;
            }
    };

    Application* Application::make(Allocator& allocator) {
        return MAKE_NEW(allocator, ApplicationImplementation);
    }

    void Application::destroy(Allocator& allocator, Application* rm) {
        MAKE_DELETE(allocator, Application, rm);
    }

    namespace application_globals {
        static Application* _application = nullptr;

        void init() {
            _application = Application::make(memory_globals::default_allocator());
        }

        void shutdown() {
            Application::destroy(memory_globals::default_allocator(), _application);
            _application = nullptr;
        }

        Application& app() {
            return *_application;
        }
    }
}
