#include "lua/lua_enviroment.h"
#include "lua/lua_stack.h"
#include "common/math/vector2.inl.h"

#include "device.h"

namespace cetech {
    static int vector2_zero(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector2(vector2::ZERO);
        return 1;
    }

    static int vector2_unit(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector2(vector2::UNIT);
        return 1;
    }

    static int vector2_up(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector2(vector2::UP);
        return 1;
    }

    static int vector2_down(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector2(vector2::DOWN);
        return 1;
    }

    static int vector2_right(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector2(vector2::RIGHT);
        return 1;
    }

    static int vector2_left(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector2(vector2::LEFT);
        return 1;
    }

    static int vector2_new(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector2(vector2::make_vector2(stack.to_float(1), stack.to_float(2)));
        return 1;
    }

    static int vector2_ctor(lua_State* L) {
        LuaStack stack(L);
        stack.remove(1);

        return vector2_new(L);
    }

    static int vector2_len(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector2::len(stack.to_vector2(1))
            );
        return 1;
    }

    static int vector2_len_inv(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector2::len_inv(stack.to_vector2(1))
            );
        return 1;
    }

    static int vector2_len_sq(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector2::len_sq(stack.to_vector2(1))
            );
        return 1;
    }

    static int vector2_distance(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector2::distance(stack.to_vector2(1), stack.to_vector2(2))
            );
        return 1;
    }

    static int vector2_distance_sq(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector2::distance_sq(stack.to_vector2(1), stack.to_vector2(2))
            );
        return 1;
    }

    static int vector2_normalized(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector2(
            vector2::normalized(stack.to_vector2(1))
            );
        return 1;
    }

    static int vector2_is_normalized(lua_State* L) {
        LuaStack stack(L);
        stack.push_bool(
            vector2::is_normalized(stack.to_vector2(1))
            );
        return 1;
    }

    static int vector2_dot(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector2::dot(stack.to_vector2(1), stack.to_vector2(2))
            );
        return 1;
    }

    static int vector2_cross(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector2::cross(stack.to_vector2(1), stack.to_vector2(2)));
        return 1;
    }

    namespace lua_vector2 {
        static const char* module_name = "Vector2";

        void load_libs(LuaEnviroment& env) {
            env.set_module_constructor(module_name, vector2_ctor);
            env.set_module_function(module_name, "new", vector2_new);

            env.set_module_function(module_name, "len", vector2_len);
            env.set_module_function(module_name, "len_inv", vector2_len_inv);
            env.set_module_function(module_name, "len_sq", vector2_len_sq);

            env.set_module_function(module_name, "distance", vector2_distance);
            env.set_module_function(module_name, "distance_sq", vector2_distance_sq);

            env.set_module_function(module_name, "normalized", vector2_normalized);
            env.set_module_function(module_name, "is_normalized", vector2_is_normalized);

            env.set_module_function(module_name, "dot", vector2_dot);
            env.set_module_function(module_name, "cross", vector2_cross);

            env.set_module_function(module_name, "zero", vector2_zero);
            env.set_module_function(module_name, "unit", vector2_unit);
            env.set_module_function(module_name, "up", vector2_up);
            env.set_module_function(module_name, "down", vector2_down);
            env.set_module_function(module_name, "right", vector2_right);
            env.set_module_function(module_name, "left", vector2_left);
        }
    }
}