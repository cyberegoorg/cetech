
#include <engine/entcom/types.h>
#include <celib/math/quatf.h>
#include "engine/lua_system/lua_system.h"

#define API_NAME "Quatf"

static int _from_axis_angle(lua_State *l) {
    quatf_t result = {0};

    const vec3f_t *axis = luasys_to_vector3(l, 1);
    f32 angle = luasys_to_f32(l, 2);

    quatf_from_axis_angle(&result, axis, angle);

    luasys_push_quat(l, result);
    return 1;
}

void _register_lua_quatf_api() {
    luasys_add_module_function(API_NAME, "from_axis_angle", _from_axis_angle);
}

