#include "cetech/lua/lua_enviroment.h"
#include "cetech/lua/lua_stack.h"

#include "celib/string/stringid.inl.h"
#include "cetech/application/application.h"
#include "cetech/package_manager/package_manager.h"

namespace cetech {

    static int package_create(lua_State* L) {
        LuaStack stack(L);

        const char* name = stack.to_string(1);
        StringId64_t package_id = stringid64::from_cstring(name);

        stack.push_string64id(package_id);
        return 1;
    }


    static int package_load(lua_State* L) {
        LuaStack stack(L);

        StringId64_t package_id = stack.to_string64id(1);
        package_manager::load(package_id);

        return 0;
    }

    static int package_unload(lua_State* L) {
        LuaStack stack(L);

        StringId64_t package_id = stack.to_string64id(1);
        package_manager::unload(package_id);

        return 0;
    }

    static int package_is_loaded(lua_State* L) {
        LuaStack stack(L);

        StringId64_t package_id = stack.to_string64id(1);
        stack.push_bool(package_manager::is_loaded(package_id));
        return 1;
    }

    static int package_flush(lua_State* L) {
        LuaStack stack(L);

        StringId64_t package_id = stack.to_string64id(1);
        package_manager::flush(package_id);
        return 0;
    }

    namespace lua_package {
        static const char* module_name = "Package";

        void load_libs() {
            lua_enviroment::set_module_function(module_name, "create", package_create);
            lua_enviroment::set_module_function(module_name, "load", package_load);
            lua_enviroment::set_module_function(module_name, "unload", package_unload);
            lua_enviroment::set_module_function(module_name, "is_loaded", package_is_loaded);
            lua_enviroment::set_module_function(module_name, "flush", package_flush);
        }
    }
}
