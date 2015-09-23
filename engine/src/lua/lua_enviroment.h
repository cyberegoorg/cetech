#pragma once

#include "lua_resource.h"

namespace cetech {
    class LuaEnviroment {
        public:
            virtual ~LuaEnviroment() {};

            virtual void execute_resource(const resource_lua::Resource* res) = 0;
            virtual void execute_string(const char* str) = 0;

            static LuaEnviroment* make(Allocator& alocator);
            static void destroy(Allocator& alocator, LuaEnviroment* le);
    };
}