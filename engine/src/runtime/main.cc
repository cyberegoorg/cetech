#include "common/memory/memory.h"
#include "common/math/math_types.h"
#include "common/math/math.h"
#include "common/math/quat.h"
#include "common/math/vector2.h"
#include "common/math/matrix33.h"
#include "common/math/matrix44.h"
#include "common/math/simd/simd.h"
#include "common/types.h"
#include "common/log/log.h"
#include "common/container/array.h"
#include "common/container/hash.h"
#include "common/container/queue.h"
#include "common/ecs/entitymanager.h"
#include "runtime/runtime.h"

#include "common/resource/resource_manager.h"
#include "common/resource/package_manager.h"


#include "resources/package.h"
#include "resources/lua.h"

#include "common/string/stringid_types.h"
#include <common/string/stringid.h>

#include "common/cvar/cvar.h"
#include "common/command_line/command_line.h"

#include "common/lua/lua_env.h"

#include "resources/lua.h"

#include "cvars/cvars.h"

#include "common/console_server/console_server.h"

#include <iostream>

#include "enet/enet.h"


using namespace cetech;
using namespace rapidjson;


void cmd_lua_execute(const rapidjson::Document& in, rapidjson::Document& out) {
    lua_enviroment::execute_string(lua_enviroment_globals::global_env(), in["args"]["script"].GetString());
}

void frame_start() {
    runtime::frame_start();
    console_server_globals::tick();
    console_server_globals::register_command("lua.execute", &cmd_lua_execute);
}

void frame_end() {
    runtime::frame_end();
}

void frame_body() {
    //log::info("frame", "frame");
}

void run() {
    while (1) {
        frame_start();
        frame_body();
        frame_end();
    }
}

void join_build_dir(char* buffer, size_t max_len, const char* basename) {
    memset(buffer, 0, max_len);

    size_t len = strlen(cvars::rm_build_dir.value_str);
    memcpy(buffer, cvars::rm_build_dir.value_str, len);

    strcpy(buffer + len, basename);
}

void load_config_json() {
    char config_path[1024] = {0};
    join_build_dir(config_path, 1024, "config.json");

    File f = runtime::file::from_file(config_path, "rb");
    const uint64_t f_sz = runtime::file::size(f);
    void* mem = memory_globals::default_allocator().allocate(f_sz + 1);
    memset(mem, 0, f_sz + 1);

    runtime::file::read(f, mem, sizeof(char), f_sz);

    rapidjson::Document document;
    document.Parse((const char*)mem);
    cvar::load_from_json(document);

    memory_globals::default_allocator().deallocate(mem);

    runtime::file::close(f);
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

    const char* source_dir = command_line::get_parameter("source-dir", 'i');
    const char* build_dir = command_line::get_parameter("build-dir", 'd');

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
    StringId64_t boot_pkg_name_h = murmur_hash_64(cvars::boot_pkg.value_str, strlen(cvars::boot_pkg.value_str), 22);
    StringId64_t boot_script_name_h =
        murmur_hash_64(cvars::boot_script.value_str, strlen(cvars::boot_script.value_str), 22);

    resource_manager::load(package_manager::type_name(), boot_pkg_name_h);
    package_manager::load(boot_pkg_name_h);

    StringId64_t lua_hash = murmur_hash_64("lua", 3, 22);

    const resource_lua::Resource* res_lua = (const resource_lua::Resource*)resource_manager::get(lua_hash,
                                                                                                 boot_script_name_h);
    lua_enviroment::execute_resource(lua_enviroment_globals::global_env(), res_lua);
}

void compile_all_resource() {
    char* files[4096] = {0};
    uint32_t files_count = 0;
    const size_t source_dir_len = strlen(cvars::rm_source_dir.value_str);

    runtime::dir::listdir(cvars::rm_source_dir.value_str, "", files, &files_count);

    for (uint32_t i = 0; i < files_count; ++i) {
        const char* path_base = files[i] + source_dir_len; /* Base path */

        resource_manager::compile(path_base);
    }
}

struct ResourceRegistration {
    StringId64_t type;

    resource_manager::resource_compiler_clb_t compiler;
    resource_manager::resource_loader_clb_t loader;
    resource_manager::resource_unloader_clb_t unloader;
};

void register_resources(const ResourceRegistration* regs) {
    const ResourceRegistration* it = regs;

    while (it->type != 0) {
        resource_manager::register_unloader(it->type, it->unloader);
        resource_manager::register_loader(it->type, it->loader);
        resource_manager::register_compiler(it->type, it->compiler);
        ++it;
    }
}

void init() {
    memory_globals::init();

    parse_command_line();

    runtime::init();

    resource_package::init();
    resource_lua::init();

    resource_manager_globals::init();
    package_manager_globals::init();

    load_config_json();

    console_server_globals::init();

    ResourceRegistration resource_regs[] = {
        {resource_package::type_hash(), & resource_package::compiler, & resource_package::loader,
         & resource_package::unloader},
        {resource_lua::type_hash(), & resource_lua::compiler, & resource_lua::loader, & resource_lua::unloader},
        {0, nullptr, nullptr, nullptr}
    };
    register_resources(resource_regs);

    if (command_line::has_argument("compile", 'c')) {
        compile_all_resource();
    }

    init_boot();

    cvar::dump_all();

    //     Window w = runtime::window::make_window(
    //         "aaa",
    //         runtime::window::WINDOWPOS_CENTERED, runtime::window::WINDOWPOS_CENTERED,
    //         800, 600,
    //         runtime::window::WINDOW_NOFLAG
    //         );
}

void shutdown() {
    package_manager_globals::shutdown();
    resource_manager_globals::shutdown();
    
    console_server_globals::shutdown();
    runtime::shutdown();
    memory_globals::shutdown();
}

int main(int argc, const char** argv) {
    command_line::set_args(argc, argv);

    init();
    run();
    shutdown();

    return 0; // TODO: error check
}
