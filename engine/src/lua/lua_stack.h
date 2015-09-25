#pragma once

#include "luajit/lua.hpp"

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


            bool get_bool(int i) {
                return lua_toboolean(_L, i) == 1;
            }

            int get_int(int i) {
                return lua_tointeger(_L, i);
            }

            float get_float(int i) {
                return (float) lua_tonumber(_L, i);
            }

            const char* get_string(int i) {
                return lua_tostring(_L, i);
            }


            lua_State* _L;
    };
}