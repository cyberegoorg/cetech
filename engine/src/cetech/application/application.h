#pragma once

#include "cetech/task_manager/task_manager.h"
#include "cetech/resource_manager/resource_manager.h"
#include "cetech/resource_compiler/resource_compiler.h"
#include "cetech/package_manager/package_manager.h"
#include "cetech/develop/develop_manager.h"
#include "cetech/develop/console_server.h"
#include "cetech/renderer/renderer.h"
#include "cetech/lua/lua_enviroment.h"

#include <inttypes.h>

namespace cetech {
    namespace application {
        enum Platform {
            PLATFORM_NONE = 0,
            PLATFORM_LINUX,
            PLATFORM_WINDOWS,
        };

        uint32_t get_frame_id();
        float get_delta_time();

        void init();
        void shutdown();
        void run();
        void quit();

        bool is_run();

        Platform platform();
    }

    namespace application_globals {
        void init();
        void shutdown();
    }
}