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

#include "common/resource_manager.h"
#include "common/package_manager.h"

#include "resources/package.h"

#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace cetech;
using namespace rapidjson;


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

void init() {
    memory_globals::init();
    runtime::init();
    resource_manager_globals::init();
    package_manager_globals::init();

    uint64_t type_h = murmur_hash_64("package", strlen("package"), 22);
    uint64_t name_h = murmur_hash_64("main", strlen("main"), 22);

    resource_manager::register_unloader(type_h, &resource_package::unloader);
    resource_manager::register_loader(type_h, &resource_package::loader);
    resource_manager::register_compiler(type_h, &resource_package::compiler);

    resource_manager::compile("main.package");
    resource_manager::load(type_h, name_h);
    
    package_manager::load(name_h);

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

int main(int argc, char** argv) {
    init();
    run();
    shutdown();

    return 0; // TODO: error check
}
