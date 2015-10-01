#include "lua/lua_enviroment.h"
#include "lua/lua_stack.h"
#include "common/math/vector3.inl.h"

#include "device.h"

namespace cetech {
    static int vector3_zero(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector3(vector3::ZERO);
        return 1;
    }

    static int vector3_unit(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector3(vector3::UNIT);
        return 1;
    }

    static int vector3_up(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector3(vector3::UP);
        return 1;
    }

    static int vector3_down(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector3(vector3::DOWN);
        return 1;
    }

    static int vector3_forwards(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector3(vector3::FORWARDS);
        return 1;
    }

    static int vector3_backwards(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector3(vector3::BACKWARDS);
        return 1;
    }

    static int vector3_right(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector3(vector3::RIGHT);
        return 1;
    }

    static int vector3_left(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector3(vector3::LEFT);
        return 1;
    }

    static int vector3_new(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector3(vector3::make_vector3(stack.to_float(1), stack.to_float(2), stack.to_float(3)));
        return 1;
    }

    static int vector3_ctor(lua_State* L) {
        LuaStack stack(L);
        stack.remove(1);

        return vector3_new(L);
    }

    static int vector3_len(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector3::len(stack.to_vector3(1))
            );
        return 1;
    }

    static int vector3_len_inv(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector3::len_inv(stack.to_vector3(1))
            );
        return 1;
    }

    static int vector3_len_sq(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector3::len_sq(stack.to_vector3(1))
            );
        return 1;
    }

    static int vector3_distance(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector3::distance(stack.to_vector3(1), stack.to_vector3(2))
            );
        return 1;
    }

    static int vector3_distance_sq(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector3::distance_sq(stack.to_vector3(1), stack.to_vector3(2))
            );
        return 1;
    }

    static int vector3_normalized(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector3(
            vector3::normalized(stack.to_vector3(1))
            );
        return 1;
    }

    static int vector3_is_normalized(lua_State* L) {
        LuaStack stack(L);
        stack.push_bool(
            vector3::is_normalized(stack.to_vector3(1))
            );
        return 1;
    }

    static int vector3_dot(lua_State* L) {
        LuaStack stack(L);
        stack.push_float(
            vector3::dot(stack.to_vector3(1), stack.to_vector3(2))
            );
        return 1;
    }

    static int vector3_cross(lua_State* L) {
        LuaStack stack(L);
        stack.push_vector3(
            vector3::cross(stack.to_vector3(1), stack.to_vector3(2))
            );
        return 1;
    }

    namespace lua_vector3 {
        static const char* module_name = "Vector3";

        void load_libs(LuaEnviroment& env) {
            env.set_module_constructor(module_name, vector3_ctor);
            env.set_module_function(module_name, "new", vector3_new);

            env.set_module_function(module_name, "len", vector3_len);
            env.set_module_function(module_name, "len_inv", vector3_len_inv);
            env.set_module_function(module_name, "len_sq", vector3_len_sq);

            env.set_module_function(module_name, "distance", vector3_distance);
            env.set_module_function(module_name, "distance_sq", vector3_distance_sq);

            env.set_module_function(module_name, "normalized", vector3_normalized);
            env.set_module_function(module_name, "is_normalized", vector3_is_normalized);

            env.set_module_function(module_name, "dot", vector3_dot);
            env.set_module_function(module_name, "cross", vector3_cross);

            env.set_module_function(module_name, "zero", vector3_zero);
            env.set_module_function(module_name, "unit", vector3_unit);
            env.set_module_function(module_name, "up", vector3_up);
            env.set_module_function(module_name, "down", vector3_down);
            env.set_module_function(module_name, "forwards", vector3_forwards);
            env.set_module_function(module_name, "backwards", vector3_backwards);
            env.set_module_function(module_name, "right", vector3_right);
            env.set_module_function(module_name, "left", vector3_left);
        }
    }
}