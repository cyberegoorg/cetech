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
#include "cetech/lua/api/lua_utils.h"
#include "cetech/lua/api/lua_log.h"


#include "celib/asserts.h"
#include "celib/log/log.h"
#include "celib/math/vector2.inl.h"
#include "celib/math/vector3.inl.h"
#include "celib/crypto/murmur_hash.inl.h"

#include "cetech/resource_manager/resource_manager.h"
#include "cetech/application/application.h"

namespace cetech {
    class LuaEnviromentImlementation : public LuaEnviroment {
        friend class LuaEnviroment;

        enum {
            MAX_TEMP_VECTOR2 = 1024,
            MAX_TEMP_VECTOR3 = 1024,
        };

        lua_State* _state;

        Vector2 _temp_vector2_buffer[MAX_TEMP_VECTOR2];
        uint32_t _temp_vector2_used;

        Vector3 _temp_vector3_buffer[MAX_TEMP_VECTOR3];
        uint32_t _temp_vector3_used;

        LuaEnviromentImlementation() : _temp_vector2_used(0), _temp_vector3_used(0) {
            _state = luaL_newstate();
            CE_CHECK_PTR(_state);

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

            lua_application::load_libs(*this);
            lua_keyboard::load_libs(*this);
            lua_mouse::load_libs(*this);

            lua_package::load_libs(*this);

            lua_vector2::load_libs(*this);
            lua_vector3::load_libs(*this);

            lua_utils::load_libs(*this);
            lua_log::load_libs(*this);
        }

        virtual ~LuaEnviromentImlementation() final {
            lua_close(_state);
        }

        virtual void execute_resource(const resource_lua::Resource* res) final {
            //lua_pushcfunction(_state, error_handler);

            luaL_loadbuffer(_state, resource_lua::get_source(res), res->size, "<unknown>");

            if (lua_pcall(_state, 0, 0, 0)) {
                const char* last_error = lua_tostring(_state, -1);
                lua_pop(_state, 1);
                log::error("lua", "%s", last_error);
            }
        }

        virtual void execute_string(const char* str) final {
            //lua_pushcfunction(_state, error_handler);

            if (luaL_dostring(_state, str)) {
                const char* last_error = lua_tostring(_state, -1);
                lua_pop(_state, 1);
                log::error("lua", "%s", last_error);
            }
        }

        virtual void set_module_function(const char* module, const char* name, const lua_CFunction func) final {
            luaL_newmetatable(this->_state, module);
            luaL_Reg entry[2];

            entry[0].name = name;
            entry[0].func = func;
            entry[1].name = NULL;
            entry[1].func = NULL;

            luaL_register(this->_state, NULL, entry);
            lua_setglobal(this->_state, module);
            lua_pop(this->_state, -1);
        }

        virtual void set_module_constructor(const char* module, const lua_CFunction func) final {
            lua_createtable(this->_state, 0, 1);
            lua_pushstring(this->_state, "__call");
            lua_pushcfunction(this->_state, func);

            lua_settable(this->_state, 1);

            lua_getglobal(this->_state, module);
            lua_pushvalue(this->_state, -2);

            lua_setmetatable(this->_state, -2);

            lua_pop(this->_state, -1);
        }

        virtual void call_global(const char* func, const char* args, ...) final {
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

        virtual void clean_temp() final {
            _temp_vector2_used = 0;
            _temp_vector3_used = 0;
        }

        // Vector2
        virtual Vector2& new_tmp_vector2() final {
            CE_ASSERT( _temp_vector2_used < MAX_TEMP_VECTOR2 );
            return _temp_vector2_buffer[_temp_vector2_used++];
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
        virtual Vector3& new_tmp_vector3() {
            CE_ASSERT( _temp_vector3_used < MAX_TEMP_VECTOR3 );
            return _temp_vector3_buffer[_temp_vector3_used++];
        }

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

        static int require(lua_State* L) {
            const char* name = lua_tostring( L, 1);
            StringId64_t name_hash = murmur_hash_64(name, strlen(name), 22);

            const resource_lua::Resource* res =
                (resource_lua::Resource*) application_globals::app().resource_manager().get(
                    resource_lua::type_hash(), name_hash);

            if (res == nullptr) {
                return 0;
            }

            luaL_loadbuffer(L, resource_lua::get_source(res), res->size, name);

            return 1;
        }

    };

    LuaEnviroment* LuaEnviroment::make(Allocator& allocator) {
        return MAKE_NEW(allocator, LuaEnviromentImlementation);
    }

    void LuaEnviroment::destroy(Allocator& allocator, LuaEnviroment* cs) {
        MAKE_DELETE(allocator, LuaEnviroment, cs);
    }

    //     namespace internal {
    //         static int error_handler(lua_State* L) {
    //             lua_getfield( L, LUA_GLOBALSINDEX, "debug");
    //             if (!lua_istable( L, -1)) {
    //                 lua_pop( L, 1);
    //                 return 0;
    //             }
    //
    //             lua_getfield( L, -1, "traceback");
    //             if (!lua_isfunction( L, -1)) {
    //                 lua_pop( L, 2);
    //                 return 0;
    //             }
    //
    //             lua_pushvalue( L, 1);
    //             lua_pushinteger( L, 2);
    //             lua_call( L, 2, 1);
    //
    //             log::error("lua", lua_tostring( L, -1));
    //
    //             lua_pop( L, 1);
    //             lua_pop( L, 1);
    //
    //             return 0;
    //         }
    //
    //     }
}