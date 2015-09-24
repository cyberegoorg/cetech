#include "device.h"

#include <unistd.h>
#include <csignal>

#include "common/memory/memory.h"
#include "common/command_line/command_line.h"
#include "common/log/handlers.h"
#include "common/container/array.inl.h"

#include "lua/lua_enviroment.h"
#include "resource/resource_manager.h"
#include "package/package_manager.h"
#include "develop/develop_manager.h"
#include "filesystem/disk_filesystem.h"
#include "package/package_resource.h"

#include "cvars/cvars.h"
#include "runtime/runtime.h"

extern "C" {
static void posix_signal_handler(int sig) {
    printf("sadsadsadsad\n");
    fflush(stdout);
    switch (sig) {

    case SIGKILL:
    case SIGINT:
        //device_globals::device().quit();
        break;

    default:
        break;
    }
}

struct sigaction sigIntHandler;
void posix_init() {
    printf("posix_init\n");

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
}

namespace cetech {
    class DeviceImplementation : public Device {
        public:
            friend class Device;

            struct {
                char run : 1;
                char pause : 1;
            } _flags;

            uint32_t _frame_id;
            uint32_t _last_frame_ticks;
            float _delta_time;

            ResourceManager* _resource_manager;
            PackageManager* _package_manager;
            DevelopManager* _develop_manager;
            ConsoleServer* _console_server;
            LuaEnviroment* _lua_eviroment;
            FileSystem* _filesystem;

            DeviceImplementation() : _frame_id(0), _last_frame_ticks(0), _delta_time(0),
                                     _resource_manager(nullptr), _package_manager(nullptr), _develop_manager(nullptr),
                                     _console_server(nullptr), _lua_eviroment(nullptr), _filesystem(nullptr) {}

            virtual float get_delta_time() const final {
                return this->_delta_time;
            }
            virtual uint32_t get_frame_id() const final {
                return this->_frame_id;
            }

            virtual void init(int argc, const char** argv) final {
                command_line_globals::set_args(argc, argv);

                log::init();
                log::register_handler(&log_handlers::stdout_handler);

                posix_init();

                _develop_manager = DevelopManager::make(memory_globals::default_allocator());
                parse_command_line();

                runtime::init();

                _filesystem = disk_filesystem::make(memory_globals::default_allocator(), cvars::rm_build_dir.value_str);
                load_config_json();

                _resource_manager = ResourceManager::make(memory_globals::default_allocator(), _filesystem);
                _package_manager = PackageManager::make(memory_globals::default_allocator());
                _console_server = ConsoleServer::make(memory_globals::default_allocator());
                _lua_eviroment = LuaEnviroment::make(memory_globals::default_allocator());


                _console_server->register_command("lua.execute", &cmd_lua_execute);

                log::register_handler(&log_handlers::console_server_handler);

                register_resources();

                if (command_line_globals::has_argument("compile", 'c')) {
                    compile_all_resource();
                }

                init_boot();

                this->_last_frame_ticks = runtime::get_ticks();
            }

            virtual void shutdown() final {
                PackageManager::destroy(memory_globals::default_allocator(), _package_manager);
                ResourceManager::destroy(memory_globals::default_allocator(), _resource_manager);
                DevelopManager::destroy(memory_globals::default_allocator(), _develop_manager);
                ConsoleServer::destroy(memory_globals::default_allocator(), _console_server);
                LuaEnviroment::destroy(memory_globals::default_allocator(), _lua_eviroment);

                runtime::shutdown();
            }

            virtual void run() final {
                if (command_line_globals::has_argument("--wait", 'w')) {
                    log::info("main", "Wating for clients.");
                    while (!_console_server->has_clients()) {
                        _console_server->tick();
                    }

                    log::debug("main", "Client connected.");
                }

                _flags.run = 1;
                float dt = 0.0f;
                while (_flags.run) {
                    _develop_manager->push_begin_frame();

                    uint32_t now_ticks = runtime::get_ticks();
                    dt = (now_ticks - this->_last_frame_ticks) * 0.001f;
                    this->_delta_time = dt;
                    this->_last_frame_ticks = now_ticks;

                    _develop_manager->push_record_float("engine.delta_time", dt);
                    _develop_manager->push_record_float("engine.frame_rate", 1.0f / dt);

                    runtime::frame_start();
                    _console_server->tick();
                    //

                    usleep(3 * 1000);

                    if (!_flags.pause) {
                        //app.update(dt)
                    }

                    //
                    runtime::frame_end();
                    _develop_manager->push_end_frame();
                    _develop_manager->send_buffer();
                    _develop_manager->clear();
                    ++(this->_frame_id);
                }

                log::info("main", "Bye Bye");
            }

            virtual void quit() final {
                _flags.run = 0;
                log::info("main", "Bye Bye!!!");
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
                device_globals::device().lua_enviroment().execute_string(in["args"]["script"].GetString());
            }

            void join_build_dir(char* buffer, size_t max_len, const char* basename) {
                memset(buffer, 0, max_len);

                size_t len = strlen(cvars::rm_build_dir.value_str);
                memcpy(buffer, cvars::rm_build_dir.value_str, len);

                strcpy(buffer + len, basename);
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

            void make_path(char* buffer, size_t max_size, const char* path) {
                memset(buffer, 0, max_size);
                strcpy(buffer, path);

                const size_t len = strlen(buffer);
                if (buffer[len - 1] != '/') {
                    buffer[len] = '/';
                }
            }

            void parse_command_line() {
                char buffer[1024] = {0};

                const char* source_dir = command_line_globals::get_parameter("source-dir", 'i');
                const char* build_dir = command_line_globals::get_parameter("build-dir", 'd');

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
                StringId64_t boot_pkg_name_h = murmur_hash_64(cvars::boot_pkg.value_str, strlen(
                                                                  cvars::boot_pkg.value_str), 22);
                StringId64_t boot_script_name_h =
                    murmur_hash_64(cvars::boot_script.value_str, strlen(cvars::boot_script.value_str), 22);

                _resource_manager->load(resource_package::type_hash(), &boot_pkg_name_h, 1);
                _package_manager->load(boot_pkg_name_h);

                StringId64_t lua_hash = murmur_hash_64("lua", 3, 22);

                const resource_lua::Resource* res_lua = (const resource_lua::Resource*)_resource_manager->get(lua_hash,
                                                                                                              boot_script_name_h);
                _lua_eviroment->execute_resource(res_lua);
            }

            void compile_all_resource() {
                Array < char* > files(memory_globals::default_allocator());

                const size_t source_dir_len = cvars::rm_source_dir.str_len;

                runtime::dir::listdir(cvars::rm_source_dir.value_str, "", files);

                FileSystem* source_fs = disk_filesystem::make(
                    memory_globals::default_allocator(), cvars::rm_source_dir.value_str);

                for (uint32_t i = 0; i < array::size(files); ++i) {
                    const char* path_base = files[i] + source_dir_len; /* Base path */

                    _resource_manager->compile(path_base, source_fs);
                }

                disk_filesystem::destroy(memory_globals::default_allocator(), source_fs);
            }
    };

    Device* Device::make(Allocator& alocator) {
        return MAKE_NEW(alocator, DeviceImplementation);
    }

    void Device::destroy(Allocator& alocator, Device* rm) {
        MAKE_DELETE(memory_globals::default_allocator(), Device, rm);
    }

    namespace device_globals {
        static Device* _device = nullptr;

        void init() {
            _device = Device::make(memory_globals::default_allocator());
        }

        void shutdown() {
            Device::destroy(memory_globals::default_allocator(), _device);
            _device = nullptr;
        }

        Device& device() {
            return *_device;
        }
    }
}
