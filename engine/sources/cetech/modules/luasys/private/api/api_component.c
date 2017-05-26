
#include <cetech/core/allocator.h>
#include <cetech/kernel/hash.h>
#include <cetech/kernel/module.h>

#include <cetech/modules/world/world.h>
#include <cetech/modules/resource/resource.h>
#include <cetech/modules/entity/entity.h>
#include <cetech/modules/component/component.h>

#include <cetech/modules/luasys/luasys.h>
#include <include/luajit/lua.h>
#include <cetech/kernel/api.h>

#define API_NAME "Component"

IMPORT_API(component_api_v0)
IMPORT_API(lua_api_v0)

static int _set_property(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t entity = {.h = luasys_to_handler(l, 2)};
    const char *type = luasys_to_string(l, 3);
    const char *key = luasys_to_string(l, 4);

    stringid64_t component_type = stringid64_from_string(type);
    stringid64_t key_id = stringid64_from_string(key);

    int val_type = luasys_value_type(l, 5);

    struct property_value value;
    switch (val_type) {
        case LUA_TNUMBER: {
            float number = luasys_to_float(l, 5);
            value = (struct property_value) {
                    .type = PROPERTY_FLOAT,
                    .value.f = number
            };
        }
            break;

        case LUA_TSTRING: {
            const char *str = luasys_to_string(l, 5);
            value = (struct property_value) {
                    .type = PROPERTY_FLOAT,
                    .value.str = str
            };
        }
            break;

        case LUA_TBOOLEAN: {
            int b = luasys_to_bool(l, 5);
            value = (struct property_value) {
                    .type = PROPERTY_FLOAT,
                    .value.b = b
            };
        }
            break;


        default:
            if (lua_api_v0.is_vec3f(l, 5)) {
                vec3f_t *v = lua_api_v0.to_vec3f(l, 5);

                value = (struct property_value) {
                        .type = PROPERTY_VEC3,
                        .value.vec3f = *v
                };
            } else if (lua_api_v0.is_quat(l, 5)) {
                quatf_t *q = lua_api_v0.to_quat(l, 5);

                value = (struct property_value) {
                        .type = PROPERTY_VEC3,
                        .value.quatf = *q
                };

            } else {
                return 0;
            }
    }

    component_api_v0.set_property(component_type, w, entity, key_id, value);

    return 0;
}

static int _get_property(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t entity = {.h = luasys_to_handler(l, 2)};
    const char *type = luasys_to_string(l, 3);
    const char *key = luasys_to_string(l, 4);

    stringid64_t component_type = stringid64_from_string(type);
    stringid64_t key_id = stringid64_from_string(key);

    struct property_value value = component_api_v0.get_property(
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


void _register_lua_component_api( struct api_v0* api) {
    GET_API(api, component_api_v0);
    GET_API(api, lua_api_v0);

    luasys_add_module_function(API_NAME, "set_property", _set_property);
    luasys_add_module_function(API_NAME, "get_property", _get_property);
}