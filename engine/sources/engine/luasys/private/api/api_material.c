
#include <engine/world/transform.h>
#include <engine/renderer/mesh_renderer.h>
#include <engine/plugin/plugin.h>
#include "../luasys.h"

#define API_NAME "Material"

static struct MaterialApiV0 MaterialApiV0;

static int _set_texture(lua_State *l) {
    material_t m = {.h = luasys_to_handler(l, 1)};
    const char *slot_name = luasys_to_string(l, 2);
    const char *texture_name = luasys_to_string(l, 3);

    MaterialApiV0.set_texture(m, slot_name,
                              stringid64_from_string(texture_name));
    return 0;
}


static int _set_vec4f(lua_State *l) {
    material_t m = {.h = luasys_to_handler(l, 1)};
    const char *slot_name = luasys_to_string(l, 2);
    cel_vec4f_t *v = luasys_to_vec4f(l, 3);

    MaterialApiV0.set_vec4f(m, slot_name, *v);

    return 0;
}

void _register_lua_material_api(get_api_fce_t get_engine_api) {
    MaterialApiV0 = *((struct MaterialApiV0 *) get_engine_api(MATERIAL_API_ID,
                                                              0));

    luasys_add_module_function(API_NAME, "set_texture", _set_texture);
    luasys_add_module_function(API_NAME, "set_vec4f", _set_vec4f);
}