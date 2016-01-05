
/*******************************************************************************
**** Includes
*******************************************************************************/

#include "celib/memory/memory.h"
#include "celib/string/stringid.inl.h"
#include "celib/command_line/command_line.h"
#include "celib/errors/errors.h"

#include "cetech/cvar/cvar.h"
#include "cetech/cvars/cvars.h"
#include "cetech/log/log.h"
#include "cetech/application/application.h"
#include "cetech/input/mouse.h"

#include "cetech/renderer/texture_resource.h"
#include "cetech/package_manager/package_resource.h"
#include "cetech/lua/lua_resource.h"
#include "cetech/resource_manager/resource_manager.h"

#include "cetech/develop/console_server.h"
#include "cetech/develop/develop_manager.h"

#include "cetech/package_manager/package_manager.h"

#include "cetech/renderer/renderer.h"
#include "cetech/lua/lua_enviroment.h"

#include <unistd.h>
#include <csignal>

using namespace cetech;

/*******************************************************************************
**** POSIX
*******************************************************************************/

#if defined(CETECH_LINUX)

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
    //sigaddset(&sigIntHandler.sa_mask, SIGABRT);
    new_action.sa_flags = 0;

    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction(SIGINT, &new_action, NULL);
    }
}
#endif

/*******************************************************************************
**** Register resource.
*******************************************************************************/
void register_resources() {
    /***************************************************************************
    **** Registertration helper
    ***************************************************************************/
    static struct ResourceRegistration {
        StringId64_t type;

        resource_compiler::resource_compiler_clb_t compiler;
        resource_manager::resource_loader_clb_t loader;
        resource_manager::resource_online_clb_t online;
        resource_manager::resource_offline_clb_t offline;
        resource_manager::resource_unloader_clb_t unloader;

    } resource_regs[] = {

        /***********************************************************************
        **** Package resource.
        ***********************************************************************/
        { resource_package::type_hash(),
          & resource_package::compile,
          & resource_package::loader,
          & resource_package::online,
          & resource_package::offline,
          & resource_package::unloader},

        /***********************************************************************
        **** Lua resource.
        ***********************************************************************/
        {resource_lua::type_hash(),
         & resource_lua::compile,
         & resource_lua::loader,
         & resource_lua::online,
         & resource_lua::offline,
         & resource_lua::unloader},

        /***********************************************************************
        **** Texture resource.
        ***********************************************************************/
        {resource_texture::type_hash(),
         & resource_texture::compile,
         & resource_texture::loader,
         & resource_texture::online,
         & resource_texture::offline,
         & resource_texture::unloader},

        /***********************************************************************
        **** LOOP BREAK.
        ***********************************************************************/
        {0, nullptr, nullptr, nullptr, nullptr, nullptr}
    };

    /***************************************************************************
    **** NOW register all resource but if defined CETECH_DEVELOP skip compiler
    **** registration
    ***************************************************************************/
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

/*******************************************************************************
**** Load config.
*******************************************************************************/
void load_config_yaml() {
    FSFile& f = filesystem::open(BUILD_DIR, "config.yaml", FSFile::READ);

    const uint64_t f_sz = f.size();
    char* mem = (char*)memory_globals::default_allocator().allocate(f_sz + 1);
    memset(mem, 0, f_sz + 1);
    f.read(mem, f_sz);
    filesystem::close(f);

    cvar::load_from_yaml(mem, f_sz);
    memory_globals::default_allocator().deallocate(mem);
}

/*******************************************************************************
**** Big init.
*******************************************************************************/
bool big_init() {

#if defined(CETECH_LINUX)
    posix_init();
#endif

    log_globals::init();
    log::register_handler(&log::stdout_handler);

    FILE* log = fopen("log.yaml", "w");
    log::register_handler(&log::file_handler, log);

    memory_globals::init();
    error_globals::init();

    filesystem_globals::init();

    char build_path[4096] = {0};
    strcat(build_path, cvars::rm_build_dir.value_str);
    strcat(build_path, cvars::compiler_platform.value_str);
    strcat(build_path, "/");

    filesystem::map_root_dir(SRC_DIR, cvars::rm_source_dir.value_str);
    filesystem::map_root_dir(BUILD_DIR, build_path);
    filesystem::map_root_dir(CORE_DIR, cvars::compiler_core_path.value_str);


#if defined(CETECH_DEVELOP)
    console_server_globals::init();
    develop_manager_globals::init();
#endif

    task_manager_globals::init();
    resource_manager_globals::init();

#if defined(CETECH_DEVELOP)
    resource_compiler_globals::init();
#endif

    register_resources();

#if defined(CETECH_DEVELOP)
    if (command_line::has_argument("compile", 'c')) {
        resource_compiler::compile_all();

        if (!command_line::has_argument("continue")) {
            return false;
        }
    }

    if (command_line::has_argument("wait", 'w')) {
        log::info("main", "Wating for clients.");

        // TODO: Rewrite wait method. rpc call.
        //         while (!console_server::has_clients()) {
        //             console_server::tick(); // TODO: spawn task in init
        //         }

        log::debug("main", "Client connected.");
    }

#endif

    load_config_yaml();

    mouse_globals::init();
    package_manager_globals::init();
    renderer_globals::init();
    lua_enviroment_globals::init();
    application_globals::init();

    return true;
}

/*******************************************************************************
**** Make path.
*******************************************************************************/
static void make_path(char* buffer,
                      size_t max_size,
                      const char* path) {
    strncpy(buffer, path, max_size);

    const size_t len = strlen(path);
    if (buffer[len - 1] != '/') {
        buffer[len] = '/';
        buffer[len+1] = '\0';
    }
}

/*******************************************************************************
**** Parse command line.
*******************************************************************************/
void parse_command_line(int argc,
                        const char** argv) {
    command_line::set_args(argc, argv);

    char buffer[1024] = {0};
    const char* build_dir = command_line::get_parameter("build-dir", 'b');
    const char* port = command_line::get_parameter("port", 'p');

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
    const char* source_dir = command_line::get_parameter("source-dir", 's');
    const char* core_dir = command_line::get_parameter("core-dir");
    
    printf("sd: %s\n", source_dir);
    
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

/*******************************************************************************
**** Big shutdown.
*******************************************************************************/
void big_shutdown() {
    package_manager_globals::shutdown();
    resource_manager_globals::shutdown();

#if defined(CETECH_DEVELOP)
    resource_compiler_globals::shutdown();
    develop_manager_globals::shutdown();
    console_server_globals::shutdown();
#endif

    application_globals::shutdown();

    lua_enviroment_globals::shutdown();

    filesystem_globals::shutdown();

    task_manager_globals::shutdown();
    error_globals::shutdown();
    memory_globals::shutdown();
    log_globals::shutdown();
}

/*******************************************************************************
**** Main.
*******************************************************************************/
int main(int argc,
         const char** argv) {

    parse_command_line(argc, argv);

    if (big_init()) {
        application::run();
    }

    big_shutdown();

    return 0; // TODO: error check
}
