
#include <cetech/celib/allocator.h>

#include <cetech/kernel/hash.h>

#include <cetech/modules/entity.h>

#include <cetech/modules/luasys.h>
#include <include/luajit/lua.h>
#include <cetech/kernel/api_system.h>
#include "../luasys_private.h"

#define API_NAME "Component"

CETECH_DECL_API(ct_component_a0)
CETECH_DECL_API(ct_lua_a0)
CETECH_DECL_API(ct_hash_a0)

static int _set_property(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_entity entity = {.h = luasys_to_handler(l, 2)};
    const char *type = luasys_to_string(l, 3);
    const char *key = luasys_to_string(l, 4);

    uint64_t component_type = ct_hash_a0.id64_from_str(type);
    uint64_t key_id = ct_hash_a0.id64_from_str(key);

    int val_type = luasys_value_type(l, 5);

    struct ct_property_value value;
    switch (val_type) {
        case LUA_TNUMBER: {
            float number = luasys_to_float(l, 5);
            value = (struct ct_property_value) {
                    .type = PROPERTY_FLOAT,
                    .value.f = number
            };
        }
            break;

        case LUA_TSTRING: {
            const char *str = luasys_to_string(l, 5);
            value = (struct ct_property_value) {
                    .type = PROPERTY_FLOAT,
                    .value.str = str
            };
        }
            break;

        case LUA_TBOOLEAN: {
            int b = luasys_to_bool(l, 5);
            value = (struct ct_property_value) {
                    .type = PROPERTY_FLOAT,
                    .value.b = b
            };
        }
            break;


        default:
            if (ct_lua_a0.is_vec3f(l, 5)) {
                vec3f_t *v = ct_lua_a0.to_vec3f(l, 5);

                value = (struct ct_property_value) {
                        .type = PROPERTY_VEC3,
                        .value.vec3f = *v
                };
            } else if (ct_lua_a0.is_quat(l, 5)) {
                quatf_t *q = ct_lua_a0.to_quat(l, 5);

                value = (struct ct_property_value) {
                        .type = PROPERTY_VEC3,
                        .value.quatf = *q
                };

            } else {
                return 0;
            }
    }

    ct_component_a0.set_property(component_type, w, entity, key_id, value);

    return 0;
}

static int _get_property(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_entity entity = {.h = luasys_to_handler(l, 2)};
    const char *type = luasys_to_string(l, 3);
    const char *key = luasys_to_string(l, 4);

    uint64_t component_type = ct_hash_a0.id64_from_str(type);
    uint64_t key_id = ct_hash_a0.id64_from_str(key);

    struct ct_property_value value = ct_component_a0.get_property(
            component_type, w, entity, key_id);

    switch (value.type) {
        case PROPERTY_BOOL:
            luasys_push_bool(l, value.value.b);
            return 1;

        case PROPERTY_FLOAT:
            luasys_push_float(l, value.value.f);
            return 1;

        case PROPERTY_STRING:
            luasys_push_string(l, value.value.str);
            return 1;

        case PROPERTY_VEC3:
            luasys_push_vec3f(l, value.value.vec3f);
            return 1;

        case PROPERTY_QUATF:
            luasys_push_quat(l, value.value.quatf);
            return 1;

        default:
            break;
    }

    return 0;
}


void _register_lua_component_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_component_a0);
    CETECH_GET_API(api, ct_lua_a0);
    CETECH_GET_API(api, ct_hash_a0);

    luasys_add_module_function(API_NAME, "set_property", _set_property);
    luasys_add_module_function(API_NAME, "get_property", _get_property);
}