#include "application.h"

#include <unistd.h>
#include <csignal>

#include "common/memory/memory.h"
#include "common/command_line/command_line.h"
#include "common/log/handlers.h"
#include "common/container/array.inl.h"
#include "common/string/stringid.inl.h"

#include "lua/lua_enviroment.h"

#include "resource/resource_manager.h"
#include "package/package_manager.h"
#include "develop/develop_manager.h"
#include "filesystem/disk_filesystem.h"
#include "package/package_resource.h"

#include "cvars/cvars.h"
#include "os/os.h"

static void posix_signal_handler(int sig) {
    switch (sig) {

    case SIGKILL:
    case SIGINT:
        cetech::application_globals::device().quit();
        break;

    default:
        break;
    }
}

struct sigaction sigIntHandler;
void posix_init() {
    struct sigaction new_action, old_action;

    new_action.sa_handler = posix_signal_handler;
    sigemptyset(&new_action.sa_mask);
    //sigaddset(&sigIntHandler.sa_mask, SIGTERM);
    new_action.sa_flags = 0;

    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction(SIGINT, &new_action, NULL);
    }
}

namespace cetech {
    class ApplicationImplementation : public Application {
        public:
            friend class Application;

            struct {
                char run : 1;
                char pause : 1;
            } _flags;

            uint32_t _frame_id;
            uint32_t _last_frame_ticks;
            float _delta_time;

            TaskManager* _task_manager;
            ResourceManager* _resource_manager;
            PackageManager* _package_manager;
            DevelopManager* _develop_manager;
            ConsoleServer* _console_server;
            LuaEnviroment* _lua_eviroment;
            FileSystem* _filesystem;

            ApplicationImplementation() : _frame_id(0), _last_frame_ticks(0), _delta_time(0),
                                     _resource_manager(nullptr), _package_manager(nullptr), _develop_manager(nullptr),
                                     _console_server(nullptr), _lua_eviroment(nullptr), _filesystem(nullptr) {}

            virtual float get_delta_time() const final {
                return this->_delta_time;
            }
            virtual uint32_t get_frame_id() const final {
                return this->_frame_id;
            }


            os::Window main_window;
            virtual void init(int argc, const char** argv) final {
                command_line_globals::set_args(argc, argv);

                log::register_handler(&log_handlers::stdout_handler);

                posix_init();

                _develop_manager = DevelopManager::make(memory_globals::default_allocator());
                parse_command_line();

                os::init();

                _filesystem = disk_filesystem::make(memory_globals::default_allocator(), cvars::rm_build_dir.value_str);
                load_config_json();

		_flags.run = 1;
		
                _task_manager = TaskManager::make(memory_globals::default_allocator());
                _resource_manager = ResourceManager::make(memory_globals::default_allocator(), _filesystem);
                _package_manager = PackageManager::make(memory_globals::default_allocator());
                _console_server = ConsoleServer::make(memory_globals::default_allocator());
                _lua_eviroment = LuaEnviroment::make(memory_globals::default_allocator());


                _console_server->register_command("lua.execute", &cmd_lua_execute);

                register_resources();

                if (command_line_globals::has_argument("compile", 'c')) {
                    compile_all_resource();
                }

                init_boot();

                this->_last_frame_ticks = os::get_ticks();

                _lua_eviroment->call_global("init");
            }

            virtual void shutdown() final {
                _lua_eviroment->call_global("shutdown");

                shutdown_boot();

                PackageManager::destroy(memory_globals::default_allocator(), _package_manager);
                ResourceManager::destroy(memory_globals::default_allocator(), _resource_manager);
                DevelopManager::destroy(memory_globals::default_allocator(), _develop_manager);
                TaskManager::destroy(memory_globals::default_allocator(), _task_manager);
                ConsoleServer::destroy(memory_globals::default_allocator(), _console_server);
                LuaEnviroment::destroy(memory_globals::default_allocator(), _lua_eviroment);
                disk_filesystem::destroy(memory_globals::default_allocator(), _filesystem);

                os::shutdown();
            }

            static void console_server_tick(void* data) {
                ApplicationImplementation* d = (ApplicationImplementation*) data;
                d->_console_server->tick();
            }

            virtual void run() final {
                if (command_line_globals::has_argument("--wait", 'w')) {
                    log::info("main", "Wating for clients.");
                    while (!_console_server->has_clients()) {
                        _console_server->tick();
                    }

                    log::debug("main", "Client connected.");
                }

                main_window = os::window::make_window(
                    "aaa",
                    os::window::WINDOWPOS_CENTERED, os::window::WINDOWPOS_CENTERED,
                    800, 600,
                    os::window::WINDOW_NOFLAG
                    );

                float dt = 0.0f;
                while (_flags.run) {
                    uint32_t now_ticks = os::get_ticks();
                    dt = (now_ticks - this->_last_frame_ticks) * 0.001f;
                    this->_delta_time = dt;
                    this->_last_frame_ticks = now_ticks;

                    _develop_manager->push_record_float("engine.frame_id", _frame_id);
                    _develop_manager->push_record_float("engine.delta_time", dt);
                    _develop_manager->push_record_float("engine.frame_rate", 1.0f / dt);

                    os::frame_start();
                    //_console_server->tick();
                    //

                    TaskManager::TaskID frame_task = _task_manager->add_empty_begin(0);
                    TaskManager::TaskID console_server_task = _task_manager->add_begin(
                        console_server_tick, this, 0,
                        NULL_TASK, frame_task
                        );

                    const TaskManager::TaskID task_end[] = {
                        frame_task,
                        console_server_task,
                    };
                    _task_manager->add_end(task_end, sizeof(task_end) / sizeof(TaskManager::TaskID));

                    usleep(3 * 1000);
                    if (!_flags.pause) {
                        _lua_eviroment->call_global("update", "f", dt);
                        _lua_eviroment->clean_temp();
                    }

                    //
                    os::frame_end();
                    _develop_manager->push_end_frame();
                    _develop_manager->send_buffer();
                    _develop_manager->clear();
                    ++(this->_frame_id);

                    _task_manager->wait(frame_task);
                }

                log::info("main", "Bye Bye");
            }

            virtual void quit() final {
                _flags.run = 0;
            }

            virtual bool is_run() final {
	      return _flags.run != 0;
	    }

            virtual TaskManager& task_manager() final {
                CE_CHECK_PTR(this->_task_manager);

                return *(this->_task_manager);
            }

            virtual ResourceManager& resource_manager() final {
                CE_CHECK_PTR(this->_resource_manager);

                return *(this->_resource_manager);
            }

            virtual PackageManager& package_manager() final {
                CE_CHECK_PTR(this->_resource_manager);

                return *(this->_package_manager);
            }

            virtual DevelopManager& develop_manager() final {
                CE_CHECK_PTR(this->_resource_manager);

                return *(this->_develop_manager);
            }

            virtual ConsoleServer& console_server() final {
                CE_CHECK_PTR(this->_resource_manager);

                return *(this->_console_server);
            }

            virtual LuaEnviroment& lua_enviroment() final {
                CE_CHECK_PTR(this->_resource_manager);

                return *(this->_lua_eviroment);
            }

            CE_INLINE void register_resources() {
                struct ResourceRegistration {
                    StringId64_t type;

                    ResourceManager::resource_compiler_clb_t compiler;
                    ResourceManager::resource_loader_clb_t loader;
                    ResourceManager::resource_unloader_clb_t unloader;
                };

                static ResourceRegistration resource_regs[] = {
                    /* package */
                    {resource_package::type_hash(), & resource_package::compiler, & resource_package::loader,
                     & resource_package::unloader},

                    /* lua */
                    {resource_lua::type_hash(), & resource_lua::compiler, & resource_lua::loader,
                     & resource_lua::unloader},

                    /* LAST */
                    {0, nullptr, nullptr, nullptr}
                };

                const ResourceRegistration* it = resource_regs;
                while (it->type != 0) {
                    _resource_manager->register_unloader(it->type, it->unloader);
                    _resource_manager->register_loader(it->type, it->loader);
                    _resource_manager->register_compiler(it->type, it->compiler);
                    ++it;
                }
            }

            static void cmd_lua_execute(const rapidjson::Document& in, rapidjson::Document& out) {
                application_globals::device().lua_enviroment().execute_string(in["args"]["script"].GetString());
            }

            void load_config_json() {
                File* f = _filesystem->open("config.json", File::READ);

                const uint64_t f_sz = f->size();
                void* mem = memory_globals::default_allocator().allocate(f_sz + 1);
                memset(mem, 0, f_sz + 1);

                f->read(mem, f_sz);

                _filesystem->close(f);

                rapidjson::Document document;
                document.Parse((const char*)mem);
                cvar::load_from_json(document);

                memory_globals::default_allocator().deallocate(mem);
            }

            static void make_path(char* buffer, size_t max_size, const char* path) {
                memset(buffer, 0, max_size);
                strcpy(buffer, path);

                const size_t len = strlen(buffer);
                if (buffer[len - 1] != '/') {
                    buffer[len] = '/';
                }
            }

            void parse_command_line() {
                char buffer[1024] = {0};

                const char* source_dir = command_line_globals::get_parameter("source-dir", 's');
                const char* build_dir = command_line_globals::get_parameter("build-dir", 'b');

                if (source_dir) {
                    make_path(buffer, 1024, source_dir);
                    cvar_internal::force_set(cvars::rm_source_dir, buffer);
                }

                if (build_dir) {
                    make_path(buffer, 1024, build_dir);
                    cvar_internal::force_set(cvars::rm_build_dir, buffer);
                }
            }

            void init_boot() {
                StringId64_t boot_pkg_name_h = stringid64::from_cstring_len(cvars::boot_pkg.value_str,
                                                                            cvars::boot_pkg.str_len);
                StringId64_t boot_script_name_h = stringid64::from_cstring_len(cvars::boot_script.value_str,
                                                                               cvars::boot_script.str_len);

                // Load boot package

                void* package_data[1];
                _resource_manager->load(package_data, resource_package::type_hash(), &boot_pkg_name_h, 1);
                _resource_manager->add_loaded(package_data, resource_package::type_hash(), &boot_pkg_name_h, 1);

                _package_manager->load(boot_pkg_name_h);
                _package_manager->flush(boot_pkg_name_h);

                // Execute boot script
                const resource_lua::Resource* res_lua;
                res_lua = (const resource_lua::Resource*) _resource_manager->get(
                    resource_lua::type_hash(), boot_script_name_h);
                _lua_eviroment->execute_resource(res_lua);
            }

            void shutdown_boot() {
                StringId64_t boot_pkg_name_h = stringid64::from_cstring_len(cvars::boot_pkg.value_str,
                                                                            cvars::boot_pkg.str_len);
                StringId64_t boot_script_name_h = stringid64::from_cstring_len(cvars::boot_script.value_str,
                                                                               cvars::boot_script.str_len);

                _package_manager->unload(boot_pkg_name_h);
                _resource_manager->unload(resource_package::type_hash(), &boot_pkg_name_h, 1);

            }

            // TODO: remove from device to other class.
            void compile_all_resource() {
                FileSystem* source_fs = disk_filesystem::make(
                    memory_globals::default_allocator(), cvars::rm_source_dir.value_str);

                TaskManager::TaskID compile_tid = _resource_manager->compile(source_fs);
                _task_manager->wait(compile_tid);

                disk_filesystem::destroy(memory_globals::default_allocator(), source_fs);
            }
    };

    Application* Application::make(Allocator& alocator) {
        return MAKE_NEW(alocator, ApplicationImplementation);
    }

    void Application::destroy(Allocator& alocator, Application* rm) {
        MAKE_DELETE(memory_globals::default_allocator(), Application, rm);
    }

    namespace application_globals {
        static Application* _device = nullptr;

        void init() {
            _device = Application::make(memory_globals::default_allocator());
        }

        void shutdown() {
            Application::destroy(memory_globals::default_allocator(), _device);
            _device = nullptr;
        }

        Application& device() {
            return *_device;
        }
    }
}
