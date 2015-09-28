#pragma once

#include "luajit/lua.hpp"
#include "common/string/stringid_types.h"

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
                lua_rawseti(_L,-2, 1);
                push_uint32(lh.l);
                lua_rawseti(_L,-2, 2);
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

            lua_State* _L;
    };
}