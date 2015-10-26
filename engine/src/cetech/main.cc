#include "celib/memory/memory.h"
#include "celib/string/stringid.inl.h"
#include "celib/command_line/command_line.h"

#include "cetech/cvar/cvar.h"
#include "cetech/cvars/cvars.h"
#include "cetech/log_system/log_system.h"
#include "cetech/application/application.h"
#include "cetech/log_system/handlers.h"
#include "cetech/filesystem/disk_filesystem.h"
#include "cetech/platform/mouse.h"
#include "cetech/os/os.h"

#include "cetech/renderer/texture/texture_resource.h"
#include "cetech/package_manager/package_resource.h"
#include "cetech/lua/lua_resource.h"


#include <unistd.h>
#include <csignal>

using namespace cetech;

#if CETECH_LINUX
void posix_signal_handler(int sig) {
    switch (sig) {
    case SIGKILL:
    case SIGINT:
        application::quit();
        break;

    default:
        break;
    }
}

void posix_init() {
    //struct sigaction sigIntHandler;
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
#endif

FileSystem* _filesystem;

void register_resources() {
    struct ResourceRegistration {
        StringId64_t type;

        resource_compiler::resource_compiler_clb_t compiler;
        resource_manager::resource_loader_clb_t loader;
        resource_manager::resource_online_clb_t online;
        resource_manager::resource_offline_clb_t offline;
        resource_manager::resource_unloader_clb_t unloader;
    };

    static ResourceRegistration resource_regs[] = {
        /* package */
        { resource_package::type_hash(),
          & resource_package::compile,
          & resource_package::loader,
          & resource_package::online,
          & resource_package::offline,
          & resource_package::unloader},

        /* lua */
        {resource_lua::type_hash(),
         & resource_lua::compile,
         & resource_lua::loader,
         & resource_lua::online,
         & resource_lua::offline,
         & resource_lua::unloader},

        /* texture */
        {resource_texture::type_hash(),
         & resource_texture::compile,
         & resource_texture::loader,
         & resource_texture::online,
         & resource_texture::offline,
         & resource_texture::unloader},

        /* LAST */
        {0, nullptr, nullptr, nullptr, nullptr, nullptr}
    };

    const ResourceRegistration* it = resource_regs;
    while (it->type != 0) {
        resource_manager::register_unloader(it->type, it->unloader);
        resource_manager::register_loader(it->type, it->loader);
        resource_manager::register_online(it->type, it->online);
        resource_manager::register_offline(it->type, it->offline);

#if defined(CETECH_DEVELOP)
        resource_compiler::register_compiler(it->type, it->compiler);
#endif
        ++it;
    }
}

void load_config_json() {
    FSFile* f = _filesystem->open("config.json", FSFile::READ);

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


bool big_init() {
#if CETECH_LINUX
    posix_init();
#endif

    memory_globals::init();

    log_globals::init();
    log_globals::log().register_handler(&log_handlers::stdout_handler);

    char build_path[4096] = {0};
    strcat(build_path, cvars::rm_build_dir.value_str);
    strcat(build_path, cvars::compiler_platform.value_str);
    _filesystem = disk_filesystem::make(memory_globals::default_allocator(), build_path);

    task_manager_globals::init();

    console_server_globals::init();
    console_server::init();
    develop_manager_globals::init();

    resource_manager_globals::init(_filesystem);

#if defined(CETECH_DEVELOP)
    resource_compiler_globals::init();
#endif

    register_resources();

#if defined(CETECH_DEVELOP)
    if (command_line_globals::has_argument("compile", 'c')) {
        resource_compiler::compile_all(_filesystem);

        if (!command_line_globals::has_argument("continue")) {
            return false;
        }
    }

    if (command_line_globals::has_argument("wait", 'w')) {
        log_globals::log().info("main", "Wating for clients.");

        while (!console_server::has_clients()) {
            console_server::tick(); // TODO: spawn task in init
        }

        log_globals::log().debug("main", "Client connected.");
    }

#endif

    load_config_json();

    os::init();
    mouse::init();

    package_manager_globals::init();

    renderer_globals::init();

    lua_enviroment_globals::init();

    application_globals::init();

    return true;
}

static void make_path(char* buffer, size_t max_size, const char* path) {
    memset(buffer, 0, max_size);
    strcpy(buffer, path);

    const size_t len = strlen(buffer);
    if (buffer[len - 1] != '/') {
        buffer[len] = '/';
    }
}


void parse_command_line(int argc, const char** argv) {
    char buffer[1024] = {0};

    command_line_globals::set_args(argc, argv);

    const char* build_dir = command_line_globals::get_parameter("build-dir", 'b');
    const char* port = command_line_globals::get_parameter("port", 'p');

    if (build_dir) {
        make_path(buffer, 1024, build_dir);
        cvar_internal::force_set(cvars::rm_build_dir, buffer);
    }

    if (port) {
        int p = 0;
        sscanf(port, "%d", &p);
        cvar_internal::force_set(cvars::console_server_port, p);
    }

#if defined(CETECH_DEVELOP)
    const char* source_dir = command_line_globals::get_parameter("source-dir", 's');
    const char* core_dir = command_line_globals::get_parameter("core-dir");

    if (source_dir) {
        make_path(buffer, 1024, source_dir);
        cvar_internal::force_set(cvars::rm_source_dir, buffer);
    }

    if (core_dir) {
        make_path(buffer, 1024, core_dir);
        cvar_internal::force_set(cvars::compiler_core_path, buffer);
    }

#endif

}

void big_shutdown() {
    renderer_globals::shutdown();

    package_manager_globals::shutdown();
    resource_manager_globals::shutdown();

    develop_manager_globals::shutdown();
    task_manager_globals::shutdown();
    console_server_globals::shutdown();

#if defined(CETECH_DEVELOP)
    resource_compiler_globals::shutdown();
#endif

    os::shutdown();

    application_globals::shutdown();

    log_globals::shutdown();
    memory_globals::shutdown();
}

int main(int argc, const char** argv) {
    parse_command_line(argc, argv);

    if (big_init()) {
        application::init();

        application::run();

        application::shutdown();
    }

    big_shutdown();

    return 0; // TODO: error check
}
