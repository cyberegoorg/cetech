#include <cetech/kernel/api_system.h>

#include <cetech/modules/entity.h>
#include <cetech/modules/transform.h>
#include <cetech/modules/camera.h>

#include "../luasys_private.h"

#define API_NAME "Camera"

CETECH_DECL_API(ct_camera_a0);

void _push_camera(lua_State *l, struct ct_camera camera) {
    uint64_t t = *((uint64_t*)&camera);
    luasys_push_uint64_t(l, t);
}

static int _transform_get(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_entity ent = {.h = luasys_to_handler(l, 2)};

    _push_camera(l, ct_camera_a0.get(w, ent));
    return 1;
}


static int _transform_has(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_entity ent = {.h = luasys_to_handler(l, 2)};

    luasys_push_bool(l, ct_camera_a0.has(w, ent));
    return 1;
}

void _register_lua_camera_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_camera_a0);

    luasys_add_module_function(API_NAME, "get", _transform_get);
    luasys_add_module_function(API_NAME, "has", _transform_has);
}