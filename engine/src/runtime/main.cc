#include "common/memory/memory.h"
#include "common/math/math_types.h"
#include "common/math/math.h"
#include "common/math/quat.h"
#include "common/math/vector2.h"
#include "common/math/matrix33.h"
#include "common/math/matrix44.h"
#include "common/math/simd/simd.h"
#include "common/types.h"
#include "common/log.h"
#include "common/container/array.h"
#include "common/container/hash.h"
#include "common/container/queue.h"
#include "common/ecs/entitymanager.h"
#include "runtime/runtime.h"

#include "common/resource/resource_manager.h"
#include "common/resource/package_manager.h"


#include "resources/package.h"

#include "common/stringid_types.h"

#include "common/cvar.h"
#include "common/command_line.h"

#include "common/cvars.h"

#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace cetech;
using namespace rapidjson;

CVar cvar1("vfloat", "desc", 1.0f, 0.0f, 5.0f);
CVar cvar2("vint", "desc", 1, 0, 5);
CVar cvar3("vstr", "desc", "ssssssss");

void frame_start() {
    runtime::frame_start();
}

void frame_end() {
    runtime::frame_end();
}

void frame_body() {
    if (runtime::mouse::button_state(runtime::mouse::button_index("left"))) {
        Vector2 m = runtime::mouse::axis();
        printf("%f, %f\n", m.x, m.y);
    }
}

void run() {
    while (1) {
        frame_start();
        frame_body();
        frame_end();
    }
}

void load_config_json() {
    File f = runtime::file::from_file("./data/build/config.json", "rb");
    const uint64_t f_sz = runtime::file::size(f);
    void* mem = memory_globals::default_allocator().allocate(f_sz + 1);
    memset(mem, 0, f_sz + 1);

    runtime::file::read(f, mem, sizeof(char), f_sz);

    rapidjson::Document document;
    document.Parse((const char*)mem);
    cvar::load_from_json(document);

    memory_globals::default_allocator().deallocate(mem);
}

void make_path(char* buffer, size_t max_size, const char* path) {
    memset(buffer, 0, max_size);
    strcpy(buffer, path);
    
    const size_t len = strlen(buffer);
    if( buffer[len - 1] != '/') {
        buffer[len] = '/';
    }
}

void parse_command_line() {
    char buffer[1024] = {0};
    
    const char* source_dir = command_line::get_parameter("source-dir", 'i');
    const char* build_dir = command_line::get_parameter("build-dir", 'd');
    
    if(source_dir) {
        make_path(buffer, 1024, source_dir);
        cvar_internal::force_set(cvars::rm_source_dir, buffer);
    }
    
    if(build_dir) {
        make_path(buffer, 1024, build_dir);
        cvar_internal::force_set(cvars::rm_build_dir, buffer);
    }
}

void init_boot() {
    uint64_t boot_pkg_name_h = murmur_hash_64(cvars::boot_pkg.value_str, strlen(cvars::boot_pkg.value_str), 22);
    uint64_t boot_script_name_h =
        murmur_hash_64(cvars::boot_script.value_str, strlen(cvars::boot_script.value_str), 22);

    resource_manager::load(package_manager::type_name(), boot_pkg_name_h);
    package_manager::load(boot_pkg_name_h);
}

void init() {
    memory_globals::init();
    
    parse_command_line();
    
    runtime::init();
    resource_manager_globals::init();
    package_manager_globals::init();

    load_config_json();

    resource_manager::register_unloader(package_manager::type_name(), &resource_package::unloader);
    resource_manager::register_loader(package_manager::type_name(), &resource_package::loader);
    resource_manager::register_compiler(package_manager::type_name(), &resource_package::compiler);

    if(command_line::has_argument("compile", 'c')) {
        resource_manager::compile(cvars::boot_pkg.value_str);
    }
    
    cvar::dump_all();

    //     Window w = runtime::window::make_window(
    //         "aaa",
    //         runtime::window::WINDOWPOS_CENTERED, runtime::window::WINDOWPOS_CENTERED,
    //         800, 600,
    //         runtime::window::WINDOW_NOFLAG
    //         );

    Matrix44 m1 = matrix44::IDENTITY;

    float det = matrix44::determinant(m1);

    printf("det: %f\n", det);
}

void shutdown() {
    runtime::shutdown();
    memory_globals::shutdown();
    resource_manager_globals::shutdown();
    package_manager_globals::shutdown();
}

int main(int argc, const char** argv) {
    command_line::set_args(argc, argv);
    
    init();
    run();
    shutdown();

    return 0; // TODO: error check
}
