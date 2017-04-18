
#include <celib/string/stringid.h>
#include <cetech/entity/entity.h>
#include <cetech/application/private/module.h>
#include <cetech/component/component.h>
#include "../luasys.h"
#include <cetech/luasys/luasys.h>

#define API_NAME "Component"

static struct ComponentSystemApiV0 ComponentSystemApiV0;
static struct LuaSysApiV0 LuaSysApiV0;

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
            f32 number = luasys_to_f32(l, 5);
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
            if (LuaSysApiV0.is_vec3f(l, 5)) {
                cel_vec3f_t *v = LuaSysApiV0.to_vec3f(l, 5);

                value = (struct property_value) {
                        .type = PROPERTY_VEC3,
                        .value.vec3f = *v
                };
            } else if (LuaSysApiV0.is_quat(l, 5)) {
                cel_quatf_t *q = LuaSysApiV0.to_quat(l, 5);

                value = (struct property_value) {
                        .type = PROPERTY_VEC3,
                        .value.quatf = *q
                };

            } else {
                return 0;
            }
    }

    ComponentSystemApiV0.set_property(component_type, w, entity, key_id, value);

    return 0;
}

static int _get_property(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t entity = {.h = luasys_to_handler(l, 2)};
    const char *type = luasys_to_string(l, 3);
    const char *key = luasys_to_string(l, 4);

    stringid64_t component_type = stringid64_from_string(type);
    stringid64_t key_id = stringid64_from_string(key);

    struct property_value value = ComponentSystemApiV0.get_property(
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


void _register_lua_component_api(get_api_fce_t get_engine_api) {
    ComponentSystemApiV0 = *((struct ComponentSystemApiV0 *) get_engine_api(
            COMPONENT_API_ID, 0));

    LuaSysApiV0 = *((struct LuaSysApiV0 *) get_engine_api(
            LUA_API_ID, 0));

    luasys_add_module_function(API_NAME, "set_property", _set_property);
    luasys_add_module_function(API_NAME, "get_property", _get_property);
}