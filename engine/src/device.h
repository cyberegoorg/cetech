#pragma once

#include "resource_manager.h"
#include "package_manager.h"
#include "develop_manager.h"
#include "console_server.h"
#include "lua/lua_enviroment.h"

#include <inttypes.h>

namespace cetech {
    class Device {
        public:
            virtual ~Device() {};

            virtual uint32_t get_frame_id() const = 0;
            virtual float get_delta_time() const = 0;

            virtual void init(int argc, const char** argv) = 0;
            virtual void shutdown() = 0;
            virtual void run() = 0;

            virtual ResourceManager& resource_manager() = 0;
            virtual PackageManager& package_manager() = 0;
            virtual DevelopManager& develop_manager() = 0;
            virtual ConsoleServer& console_server() = 0;
            virtual LuaEnviroment& lua_enviroment() = 0;

            static Device* make(Allocator& alocator);
            static void destroy(Allocator& alocator, Device* rm);
    };

    namespace device_globals {
        void init();
        void shutdown();

        Device& device();
    }
}