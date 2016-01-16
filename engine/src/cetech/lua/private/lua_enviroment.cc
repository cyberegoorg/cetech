#include <cstring>

#include "luajit/lua.hpp"

#include "cetech/lua/lua_enviroment.h"
#include "cetech/lua/lua_stack.h"

#include "cetech/lua/api/lua_mouse.h"
#include "cetech/lua/api/lua_application.h"
#include "cetech/lua/api/lua_keyboard.h"
#include "cetech/lua/api/lua_package.h"
#include "cetech/lua/api/lua_vector2.h"
#include "cetech/lua/api/lua_vector3.h"
#include "cetech/lua/api/lua_log.h"


#include "celib/errors/errors.h"
#include "celib/log/log.h"
#include "celib/math/vector2.inl.h"
#include "celib/math/vector3.inl.h"
#include "celib/crypto/murmur_hash.inl.h"

#include "cetech/resource_manager/resource_manager.h"
#include "cetech/application/application.h"

namespace cetech {
    namespace {
        using namespace lua_enviroment;

        enum {
            MAX_TEMP_VECTOR2 = 1024,
            MAX_TEMP_VECTOR3 = 1024,
        };

        struct LuaEnviromentData {
            lua_State* _state;

            Vector2 _temp_vector2_buffer[MAX_TEMP_VECTOR2];
            uint32_t _temp_vector2_used;

            Vector3 _temp_vector3_buffer[MAX_TEMP_VECTOR3];
            uint32_t _temp_vector3_used;
        };

        struct Globals {
            static const int MEMORY = sizeof(LuaEnviromentData);
            char buffer[MEMORY];

            LuaEnviromentData* data;

            Globals() : buffer {
                0
            }, data(0) {}
        } _globals;


        static int require(lua_State* L) {
            const char* name = lua_tostring( L, 1);
            StringId64_t name_hash = murmur_hash_64(name, strlen(name), 22);

            const resource_lua::Resource* res =
                (resource_lua::Resource*) resource_manager::get(
                    resource_lua::type_hash(), name_hash);

            if (res == nullptr) {
                return 0;
            }

            luaL_loadbuffer(L, resource_lua::get_source(res), res->size, name);

            return 1;
        }

        static int Vector2_add(lua_State* L) {
            LuaStack stack(L);
            const Vector2& a = stack.to_vector2(1);
            const Vector2& b = stack.to_vector2(2);
            stack.push_vector2(a + b);
            return 1;
        }
        static int Vector2_sub(lua_State* L) {
            LuaStack stack(L);
            const Vector2& a = stack.to_vector2(1);
            const Vector2& b = stack.to_vector2(2);
            stack.push_vector2(a - b);
            return 1;
        }
        static int Vector2_mul(lua_State* L) {
            LuaStack stack(L);
            const Vector2& a = stack.to_vector2(1);
            const float b = stack.to_float(2);
            stack.push_vector2(a * b);
            return 1;
        }
        static int Vector2_div(lua_State* L) {
            LuaStack stack(L);
            const Vector2& a = stack.to_vector2(1);
            const float b = stack.to_float(2);
            stack.push_vector2(a / b);
            return 1;
        }
        static int Vector2_unm(lua_State* L) {
            LuaStack stack(L);
            stack.push_vector2(-stack.to_vector2(1));
            return 1;
        }
        static int Vector2_index(lua_State* L) {
            LuaStack stack(L);
            Vector2& v = stack.to_vector2(1);
            const char* s = stack.to_string(2);
            switch (s[0])
            {
            case 'x': stack.push_float(v.x); return 1;
            case 'y': stack.push_float(v.y); return 1;
            default: log::error("lua", "Vector2 bad index '%s'", s[0]); break;
            }

            return 0;
        }
        static int Vector2_newindex(lua_State* L) {
            LuaStack stack(L);
            Vector2& v = stack.to_vector2(1);
            const char* s = stack.to_string(2);
            const float value = stack.to_float(3);

            switch (s[0])
            {
            case 'x': v.x = value; break;
            case 'y': v.y = value; break;
            default: log::error("lua", "Vector2 bad index '%s'", s[0]); break;
            }

            return 0;
        }

        void create_Vector2_mt() {
            lua_State* _state = _globals.data->_state;

            luaL_newmetatable(_state, "Vector2_mt");

            lua_pushstring(_state, "__add");
            lua_pushcfunction(_state, Vector2_add);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__sub");
            lua_pushcfunction(_state, Vector2_sub);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__mul");
            lua_pushcfunction(_state, Vector2_mul);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__div");
            lua_pushcfunction(_state, Vector2_div);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__unm");
            lua_pushcfunction(_state, Vector2_unm);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__index");
            lua_pushcfunction(_state, Vector2_index);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__newindex");
            lua_pushcfunction(_state, Vector2_newindex);
            lua_settable(_state, 1);

            lua_pop(_state, 1);
        }
        // -- Vector2

        // Vector3

        static int Vector3_add(lua_State* L) {
            LuaStack stack(L);
            const Vector3& a = stack.to_vector3(1);
            const Vector3& b = stack.to_vector3(2);
            stack.push_vector3(a + b);
            return 1;
        }
        static int Vector3_sub(lua_State* L) {
            LuaStack stack(L);
            const Vector3& a = stack.to_vector3(1);
            const Vector3& b = stack.to_vector3(2);
            stack.push_vector3(a - b);
            return 1;
        }
        static int Vector3_mul(lua_State* L) {
            LuaStack stack(L);
            const Vector3& a = stack.to_vector3(1);
            const float b = stack.to_float(2);
            stack.push_vector3(a * b);
            return 1;
        }
        static int Vector3_div(lua_State* L) {
            LuaStack stack(L);
            const Vector3& a = stack.to_vector3(1);
            const float b = stack.to_float(2);
            stack.push_vector3(a / b);
            return 1;
        }
        static int Vector3_unm(lua_State* L) {
            LuaStack stack(L);
            stack.push_vector3(-stack.to_vector3(1));
            return 1;
        }
        static int Vector3_index(lua_State* L) {
            LuaStack stack(L);
            Vector3& v = stack.to_vector3(1);
            const char* s = stack.to_string(2);
            switch (s[0])
            {
            case 'x': stack.push_float(v.x); return 1;
            case 'y': stack.push_float(v.y); return 1;
            case 'z': stack.push_float(v.z); return 1;
            default: log::error("lua", "Vector3 bad index '%s'", s[0]); break;
            }

            return 0;
        }
        static int Vector3_newindex(lua_State* L) {
            LuaStack stack(L);
            Vector3& v = stack.to_vector3(1);
            const char* s = stack.to_string(2);
            const float value = stack.to_float(3);

            switch (s[0])
            {
            case 'x': v.x = value; break;
            case 'y': v.y = value; break;
            case 'z': v.z = value; break;
            default: log::error("lua", "Vector3 bad index '%s'", s[0]); break;
            }

            return 0;
        }

        void create_Vector3_mt() {
            lua_State* _state = _globals.data->_state;

            luaL_newmetatable(_state, "Vector3_mt");

            lua_pushstring(_state, "__add");
            lua_pushcfunction(_state, Vector3_add);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__sub");
            lua_pushcfunction(_state, Vector3_sub);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__mul");
            lua_pushcfunction(_state, Vector3_mul);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__div");
            lua_pushcfunction(_state, Vector3_div);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__unm");
            lua_pushcfunction(_state, Vector3_unm);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__index");
            lua_pushcfunction(_state, Vector3_index);
            lua_settable(_state, 1);

            lua_pushstring(_state, "__newindex");
            lua_pushcfunction(_state, Vector3_newindex);
            lua_settable(_state, 1);

            lua_pop(_state, 1);
        }
        // -- Vector3

#if defined(CETECH_DEVELOP)
        static void cmd_lua_execute(const mpack_node_t& root) {
            char script[4096];
            mpack_node_copy_cstr(mpack_node_map_cstr(mpack_node_map_cstr(root, "args"), "script"), script, 4096);
            lua_enviroment::execute_string(script);
        }
#endif
    }

    namespace lua_enviroment {
        void init() {
            LuaEnviromentData* data = _globals.data;

#if defined(CETECH_DEVELOP)
            console_server::register_command("lua.execute", &cmd_lua_execute);
#endif

            lua_State* _state = luaL_newstate();

            CE_ASSERT("lua_enviroment", _state != nullptr);

            data->_state = _state;


            luaL_openlibs(_state);

            lua_getfield(_state, LUA_GLOBALSINDEX, "package");
            lua_getfield(_state, -1, "loaders");
            lua_remove(_state, -2);

            int num_loaders = 0;
            lua_pushnil(_state);
            while (lua_next(_state, -2) != 0) {
                lua_pop(_state, 1);
                num_loaders++;
            }

            lua_pushinteger(_state, num_loaders + 1);
            lua_pushcfunction(_state, require);
            lua_rawset(_state, -3);
            lua_pop(_state, 1);

            create_Vector2_mt();
            create_Vector3_mt();

            lua_application::load_libs();
            lua_keyboard::load_libs();
            lua_mouse::load_libs();

            lua_package::load_libs();

            lua_vector2::load_libs();
            lua_vector3::load_libs();

            lua_log::load_libs();
        }

        void shutdown() {
            lua_close(_globals.data->_state);
        }

        void execute_resource(const resource_lua::Resource* res) {
            lua_State* _state = _globals.data->_state;
            //lua_pushcfunction(_state, error_handler);


            luaL_loadbuffer(_state, resource_lua::get_source(res), res->size, "<unknown>");

            if (lua_pcall(_state, 0, 0, 0)) {
                const char* last_error = lua_tostring(_state, -1);
                lua_pop(_state, 1);
                log::error("lua", "%s", last_error);
            }
        }

        void execute_string(const char* str) {
            lua_State* _state = _globals.data->_state;
            //lua_pushcfunction(_state, error_handler);

            if (luaL_dostring(_state, str)) {
                const char* last_error = lua_tostring(_state, -1);
                lua_pop(_state, 1);
                log::error("lua", "%s", last_error);
            }
        }

        void set_module_function(const char* module,
                                 const char* name,
                                 const lua_CFunction func) {
            lua_State* _state = _globals.data->_state;

            luaL_newmetatable(_state, module);
            luaL_Reg entry[2];

            entry[0].name = name;
            entry[0].func = func;
            entry[1].name = NULL;
            entry[1].func = NULL;

            luaL_register(_state, NULL, entry);
            lua_setglobal(_state, module);
            lua_pop(_state, -1);
        }

        void set_module_constructor(const char* module,
                                    const lua_CFunction func) {
            lua_State* _state = _globals.data->_state;

            lua_createtable(_state, 0, 1);
            lua_pushstring(_state, "__call");
            lua_pushcfunction(_state, func);

            lua_settable(_state, 1);

            lua_getglobal(_state, module);
            lua_pushvalue(_state, -2);

            lua_setmetatable(_state, -2);

            lua_pop(_state, -1);
        }

        void call_global(const char* func,
                         const char* args,
                         ...) {
            lua_State* _state = _globals.data->_state;

            LuaStack stack(_state);
            uint32_t argc = 0;

            //lua_pushcfunction(L, error_handler);
            lua_getglobal(_state, func);

            if (args != nullptr) {
                va_list vl;
                va_start(vl, args);

                const char* it = args;
                while (*it != '\0') {
                    switch (*it) {
                    case 'i':
                        stack.push_int32(va_arg(vl, int32_t));
                        break;

                    case 'u':
                        stack.push_uint32(va_arg(vl, uint32_t));
                        break;

                    case 'f':
                        stack.push_float(va_arg(vl, double));
                        break;
                    }

                    ++argc;
                    ++it;
                }

                va_end(vl);
            }

            lua_pcall(_state, argc, 0, -argc - 2);
            lua_pop(_state, -1);
        }

        void clean_temp() {
            LuaEnviromentData* data = _globals.data;

            data->_temp_vector2_used = 0;
            data->_temp_vector3_used = 0;
        }

        // Vector2
        Vector2& new_tmp_vector2() {
            LuaEnviromentData* data = _globals.data;

            CE_ASSERT( "lua_enviroment", data->_temp_vector2_used < MAX_TEMP_VECTOR2 );
            return data->_temp_vector2_buffer[data->_temp_vector2_used++];
        }

        Vector3& new_tmp_vector3() {
            LuaEnviromentData* data = _globals.data;

            CE_ASSERT( "lua_enviroment", data->_temp_vector3_used < MAX_TEMP_VECTOR3 );
            return data->_temp_vector3_buffer[data->_temp_vector3_used++];
        }

    }

    namespace lua_enviroment_globals {
        void init() {
            log::info("lua_enviroment_globals", "Init");

            char* p = _globals.buffer;
            _globals.data = new(p) LuaEnviromentData();

            lua_enviroment::init();
        }

        void shutdown() {
            log::info("lua_enviroment_globals", "Shutdown");

            lua_enviroment::shutdown();
            _globals.data->~LuaEnviromentData();
            _globals = Globals();
        }
    }
}
