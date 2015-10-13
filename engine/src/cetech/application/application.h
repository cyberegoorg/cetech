#pragma once

#include "cetech/task_manager/task_manager.h"
#include "cetech/resource/resource_manager.h"
#include "cetech/resource/resource_compiler.h"
#include "cetech/package/package_manager.h"
#include "cetech/develop/develop_manager.h"
#include "cetech/develop/console_server.h"
#include "cetech/renderer/renderer.h"
#include "cetech/lua/lua_enviroment.h"

#include <inttypes.h>

namespace cetech {
    class Application {
        public:
            virtual ~Application() {};

            virtual uint32_t get_frame_id() const = 0;
            virtual float get_delta_time() const = 0;

            virtual void init(int argc, const char** argv) = 0;
            virtual void shutdown() = 0;
            virtual void run() = 0;
            virtual void quit() = 0;

	    virtual void resize(uint32_t w, uint32_t h) = 0;
	    
            virtual bool is_run() = 0;

            virtual TaskManager& task_manager() = 0;
            virtual ResourceManager& resource_manager() = 0;
            virtual ResourceCompiler& resource_compiler() = 0;
            virtual PackageManager& package_manager() = 0;
            virtual DevelopManager& develop_manager() = 0;
            virtual ConsoleServer& console_server() = 0;
            virtual LuaEnviroment& lua_enviroment() = 0;
	    virtual Renderer& renderer() = 0;


            static Application* make(Allocator& allocator);
            static void destroy(Allocator& allocator, Application* rm);
    };

    namespace application_globals {
        void init();
        void shutdown();

        Application& app();
    }
}