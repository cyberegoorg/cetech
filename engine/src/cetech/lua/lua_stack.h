#pragma once

#include "luajit/lua.hpp"
#include "celib/string/stringid_types.h"
#include "celib/math/math_types.h"
#include "cetech/application/application.h"

namespace cetech {
    class LuaStack {
        public:
            LuaStack(lua_State * L) : _L(L) {}

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
                Vector2& tmp_v = application_globals::app().lua_enviroment().new_tmp_vector2();
                tmp_v = v;

                lua_pushlightuserdata(_L, &tmp_v);
                luaL_getmetatable(_L, "Vector2_mt");
                lua_setmetatable(_L, -2);
            }

            void push_vector3(const Vector3& v) {
                Vector3& tmp_v = application_globals::app().lua_enviroment().new_tmp_vector3();
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

            void to_json(int i, rapidjson::Document& document, rapidjson::Value& root) {
		lua_pushnil(_L);
                while (lua_next(_L, i) != 0) {
                    const char* key = lua_tostring(_L, -2);
		    rapidjson::Value rapid_key(key, strlen(key), document.GetAllocator());

		    int type = lua_type(_L, -1);

		    switch (type) {
                    case LUA_TNUMBER: {
			  uint32_t number = lua_tonumber(_L,-1);
			  root.AddMember(rapid_key, number, document.GetAllocator());
			}
                        break;
                    case LUA_TSTRING: {
			const char* str = lua_tostring(_L,-1);
			root.AddMember(rapid_key, rapidjson::Value(str, strlen(str), document.GetAllocator()), document.GetAllocator());
			}
                        break;
                    case LUA_TBOOLEAN: {
			bool b = lua_toboolean(_L,-1);
			root.AddMember(rapid_key, rapidjson::Value(b), document.GetAllocator());
			}
                        break;
                    case LUA_TNIL:{
			root.AddMember(rapid_key, rapidjson::Value(rapidjson::kNullType), document.GetAllocator());
			}
                        break;
			
                    case LUA_TTABLE:{
			rapidjson::Value v(rapidjson::kObjectType);
			to_json(lua_gettop(_L), document, v);
			root.AddMember(rapid_key, v, document.GetAllocator());
		    }
                        break;
			
		    case LUA_TFUNCTION: {
			const char* str = lua_typename(_L,-1);
			root.AddMember(rapid_key, "function", document.GetAllocator());
			}
			
                    }

                    lua_pop(_L, 1);
                }
            }

            lua_State* _L;
    };
}