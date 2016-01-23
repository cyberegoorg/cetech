#pragma once

#include "luajit/lua.hpp"
#include "mpack/mpack.h"

#include "celib/string/types.h"
#include "celib/math/types.h"
#include "celib/macros.h"
#include "celib/container/array.inl.h"

#include "cetech/application/application.h"
#include "celib/log/log.h"

namespace cetech {
    class LuaStack {
        public:
            explicit LuaStack(lua_State* L) : _L(L) {}

            int num_args() {
                return lua_gettop(_L);
            }

            void remove(int i) {
                lua_remove(_L, i);
            }

            void pop(int n) {
                lua_pop(_L, n);
            }

            bool is_nil(int i) {
                return lua_isnil(_L, i) == 1;
            }

            bool is_number(int i) {
                return lua_isnumber(_L, i) == 1;
            }

            int value_type(int i) {
                return lua_type(_L, i);
            }

            void push_nil() {
                lua_pushnil(_L);
            }

            void push_bool(bool value) {
                lua_pushboolean(_L, value);
            }

            void push_int32(int32_t value) {
                lua_pushinteger(_L, value);
            }

            void push_uint32(uint32_t value) {
                lua_pushinteger(_L, value);
            }

            void push_float(float value) {
                lua_pushnumber(_L, value);
            }

            void push_string(const char* s) {
                lua_pushstring(_L, s);
            }

            void push_string64id(const StringId64_t string_id) {
                union {
                    StringId64_t lh;
                    struct {
                        uint32_t l;
                        uint32_t h;
                    };
                } lh;
                lh.lh = string_id;

                lua_newtable(_L);

                push_uint32(lh.h);
                lua_rawseti(_L, -2, 1);
                push_uint32(lh.l);
                lua_rawseti(_L, -2, 2);
            }

            void push_vector2(const Vector2& v) {
                Vector2& tmp_v = lua_enviroment::new_tmp_vector2();
                tmp_v = v;

                lua_pushlightuserdata(_L, &tmp_v);
                luaL_getmetatable(_L, "Vector2_mt");
                lua_setmetatable(_L, -2);
            }

            void push_vector3(const Vector3& v) {
                Vector3& tmp_v = lua_enviroment::new_tmp_vector3();
                tmp_v = v;

                lua_pushlightuserdata(_L, &tmp_v);
                luaL_getmetatable(_L, "Vector3_mt");
                lua_setmetatable(_L, -2);
            }

            bool to_bool(int i) {
                return lua_toboolean(_L, i) == 1;
            }

            int to_int(int i) {
                return lua_tointeger(_L, i);
            }

            float to_float(int i) {
                return (float) lua_tonumber(_L, i);
            }

            const char* to_string(int i) {
                return lua_tostring(_L, i);
            }

            const char* to_string(int i,
                                  size_t* len) {
                return lua_tolstring(_L, i, len);
            }

            StringId64_t to_string64id(int i) {
                union {
                    StringId64_t lh;
                    struct {
                        uint32_t l;
                        uint32_t h;
                    };
                } lh;


                lua_rawgeti(_L, i, 1);
                lh.h = to_int(-1);

                lua_rawgeti(_L, i, 2);
                lh.l = to_int(-1);

                return lh.lh;
            }

            Vector2& to_vector2(int i) {
                void* v = lua_touserdata(_L, i);
                return *(Vector2*)v;
            }

            Vector3& to_vector3(int i) {
                void* v = lua_touserdata(_L, i);
                return *(Vector3*)v;
            }

            CE_INLINE void push_indent(Array < char >& output,
                                       const uint32_t level) {
                for (uint32_t i = 0; i < level; ++i) {
                    array::push(output, CSTR_TO_ARG("  "));
                }
            }

            void to_mpack(const int i,
                          mpack_writer_t& writer,
                          uint32_t level) {

                int type = lua_type(_L, i);

                switch (type) {
                case LUA_TNUMBER: {
                    uint32_t number = lua_tonumber(_L, i);
                    mpack_write_i32(&writer, number);
                }
                break;

                case LUA_TSTRING: {
                    const char* str = lua_tostring(_L, i);
                    mpack_write_cstr(&writer, str);
                }
                break;

                case LUA_TBOOLEAN: {
                    bool b = lua_toboolean(_L, i);
                    mpack_write_bool(&writer, b);
                }
                break;

                case LUA_TNIL: {
                    mpack_write_nil(&writer);
                }
                break;

                case LUA_TTABLE: {
                    size_t count = 0;

                    for (lua_pushnil(_L); lua_next(_L, -2); lua_pop(_L, 1)) {
                        ++count;
                    }

                    mpack_start_map(&writer, count);

                    for (lua_pushnil(_L); lua_next(_L, -2); lua_pop(_L, 1)) {
                        const char* key = lua_tostring(_L, -2);

                        mpack_write_cstr(&writer, key);
                        to_mpack(lua_gettop(_L), writer, level + 1);
                    }

                    mpack_finish_map(&writer);
                }
                break;

                case LUA_TFUNCTION: {
                    mpack_write_cstr(&writer, "function");
                }
                break;

                }
            }

            void to_yaml(const int i,
                         Array < char >& output,
                         uint32_t level) {

                lua_pushnil(_L);
                while (lua_next(_L, i) != 0) {
                    const char* key = lua_tostring(_L, -2);

                    push_indent(output, level);
                    array::push(output, STR_TO_ARG(key));
                    array::push(output, CSTR_TO_ARG(": "));

                    int type = lua_type(_L, -1);

                    switch (type) {
                    case LUA_TNUMBER: {
                        char number_str[64];
                        uint32_t number = lua_tonumber(_L, -1);

                        sprintf(number_str, "%d", number);

                        array::push(output, STR_TO_ARG(number_str));
                        array::push(output, CSTR_TO_ARG("\n"));
                    }
                    break;
                    case LUA_TSTRING: {
                        const char* str = lua_tostring(_L, -1);

                        array::push(output, STR_TO_ARG(str));
                        array::push(output, CSTR_TO_ARG("\n"));
                    }
                    break;
                    case LUA_TBOOLEAN: {
                        bool b = lua_toboolean(_L, -1);

                        array::push(output, CSTR_TO_ARG(b ? "True" : "False"));
                        array::push(output, CSTR_TO_ARG("\n"));
                    }
                    break;
                    case LUA_TNIL: {
                        array::push(output, CSTR_TO_ARG("~\n"));
                    }
                    break;

                    case LUA_TTABLE: {
                        array::push(output, CSTR_TO_ARG("\n"));
                        to_yaml(lua_gettop(_L), output, level + 1);
                    }
                    break;

                    case LUA_TFUNCTION: {
                        array::push(output, CSTR_TO_ARG("function\n"));
                    }

                    }

                    lua_pop(_L, 1);
                }
            }


            void load_string(const char* string) {
                luaL_loadstring(_L, string);
            }

            void execute_string(const char* string) {
                if (luaL_dostring(_L, string)) {
                    const char* last_error = lua_tostring(_L, -1);
                    lua_pop(_L, 1);
                    log::error("lua", "%s", last_error);
                }
            }

            lua_State* _L;
    };
}
