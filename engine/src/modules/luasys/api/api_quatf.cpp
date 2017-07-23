
#include <cetech/modules/luasys.h>

#include <cetech/kernel/api_system.h>
#include <cetech/celib/fpumath.h>
#include "../luasys_private.h"

#define API_NAME "Quatf"

static int _ctor(lua_State *l) {
    float x = luasys_to_float(l, 1);
    float y = luasys_to_float(l, 2);
    float z = luasys_to_float(l, 3);
    float w = luasys_to_float(l, 4);

    luasys_push_quat(l, (float[4]) {x, y, z, w});
    return 1;
}

static int _is(lua_State *l) {
    luasys_push_bool(l, _is_quat(l, 1));
    return 1;
}

static int _from_axis_angle(lua_State *l) {
    float result[4] = {0};
    float axis[3] = {0};

    luasys_to_vec3f(l, 1, axis);
    float angle = luasys_to_float(l, 2) * celib::DEG_TO_RAD;

    celib::quat_rotate_axis(result, axis, angle);

    luasys_push_quat(l, result);
    return 1;
}

static int _from_euler(lua_State *l) {
    float result[4] = {0};

    const float heading = luasys_to_float(l, 1)* celib::DEG_TO_RAD;
    const float attitude = luasys_to_float(l, 2)* celib::DEG_TO_RAD;
    const float bank = luasys_to_float(l, 3)* celib::DEG_TO_RAD;

    celib::quatFromEuler(result, heading, attitude, bank);

    luasys_push_quat(l, result);
    return 1;
}

static int _to_mat44f(lua_State *l) {
    float q[4];
    float m[16];

    luasys_to_quat(l, 1, q);

    celib::mat4_quat(m, q);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _to_euler_angle(lua_State *l) {
    float q[4];
    float v[3];
    float v_deg[3];

    luasys_to_quat(l, 1, q);

    celib::quat_to_euler(v, q);

    celib::vec3_mul(v_deg, v, celib::RAD_TO_DEG);

    luasys_push_vec3f(l, v_deg);
    return 1;
}

static int _normalized(lua_State *l) {
    float q[4];
    float res[4];

    celib::quat_norm(res, q);

    luasys_push_quat(l, res);
    return 1;
}

void _register_lua_quatf_api(struct ct_api_a0 *api) {
    luasys_add_module_function(API_NAME, "make", _ctor);
    luasys_add_module_function(API_NAME, "is", _is);
    luasys_add_module_function(API_NAME, "from_axis_angle", _from_axis_angle);
    luasys_add_module_function(API_NAME, "from_euler", _from_euler);

    luasys_add_module_function(API_NAME, "to_mat44f", _to_mat44f);
    luasys_add_module_function(API_NAME, "to_euler_angle", _to_euler_angle);

    luasys_add_module_function(API_NAME, "normalized", _normalized);
}

