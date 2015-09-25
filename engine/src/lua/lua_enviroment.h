#pragma once

#include "luajit/lua.hpp"
#include "lua_resource.h"

namespace cetech {
    class LuaEnviroment {
        public:
            virtual ~LuaEnviroment() {};

            virtual void execute_resource(const resource_lua::Resource* res) = 0;
            virtual void execute_string(const char* str) = 0;

            virtual void set_module_function(const char* module, const char* name, const lua_CFunction func) = 0;
            virtual void set_module_constructor(const char* module, const lua_CFunction func) = 0;

            virtual void call_global(const char* func, const char* args = nullptr, ...) = 0;
            
            static LuaEnviroment* make(Allocator& alocator);
            static void destroy(Allocator& alocator, LuaEnviroment* le);
    };
}