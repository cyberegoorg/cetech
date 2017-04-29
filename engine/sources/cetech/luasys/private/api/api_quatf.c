#include <stddef.h>
#include <cetech/memory/allocator.h>
#include <cetech/math/math_quatf.h>
#include "../luasys.h"
#include <cetech/module/module.h>

#define API_NAME "Quatf"

static int _from_axis_angle(lua_State *l) {
    cel_quatf_t result = {0};

    const cel_vec3f_t *axis = luasys_to_vec3f(l, 1);
    float angle = luasys_to_float(l, 2);

    cel_quatf_from_axis_angle(&result, axis, angle);

    luasys_push_quat(l, result);
    return 1;
}

static int _from_euler(lua_State *l) {
    cel_quatf_t result = {0};

    const float heading = luasys_to_float(l, 1);
    const float attitude = luasys_to_float(l, 2);
    const float bank = luasys_to_float(l, 3);

    cel_quatf_from_euler(&result, heading, attitude, bank);

    luasys_push_quat(l, result);
    return 1;
}

static int _to_mat44f(lua_State *l) {
    const cel_quatf_t *q = luasys_to_quat(l, 1);
    cel_mat44f_t result = {0};

    cel_quatf_to_mat44f(&result, q);

    luasys_push_mat44f(l, result);
    return 1;
}

static int _to_euler_angle(lua_State *l) {
    const cel_quatf_t *q = luasys_to_quat(l, 1);
    cel_vec3f_t result = {0};

    cel_quatf_to_eurel_angle(&result, q);

    luasys_push_vec3f(l, result);
    return 1;
}

static int _length(lua_State *l) {
    cel_quatf_t *v = luasys_to_quat(l, 1);
    luasys_push_float(l, cel_quatf_length(v));
    return 1;
}

static int _length_squared(lua_State *l) {
    cel_quatf_t *v = luasys_to_quat(l, 1);
    luasys_push_float(l, cel_quatf_length_squared(v));
    return 1;
}

static int _normalized(lua_State *l) {
    cel_quatf_t *v = luasys_to_quat(l, 1);
    cel_quatf_t res = {0};

    cel_quatf_normalized(&res, v);

    luasys_push_quat(l, res);
    return 1;
}

void _register_lua_quatf_api(get_api_fce_t get_engine_api) {
    luasys_add_module_function(API_NAME, "from_axis_angle", _from_axis_angle);
    luasys_add_module_function(API_NAME, "from_euler", _from_euler);

    luasys_add_module_function(API_NAME, "to_mat44f", _to_mat44f);
    luasys_add_module_function(API_NAME, "to_euler_angle", _to_euler_angle);

    luasys_add_module_function(API_NAME, "length", _length);
    luasys_add_module_function(API_NAME, "length_squared", _length_squared);
    luasys_add_module_function(API_NAME, "normalized", _normalized);
}

