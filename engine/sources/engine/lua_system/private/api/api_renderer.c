#include "engine/application/application.h"
#include "engine/renderer/renderer.h"
#include "engine/lua_system/lua_system.h"

#define API_NAME "Renderer"

static int _renderer_render_world(lua_State *l) {
    renderer_render_world(0, 0, 0);
    return 0;
}

static int _renderer_set_debug(lua_State *l) {
    int debug = luasys_to_bool(l, 1);
    renderer_set_debug(debug);
    return 0;
}

void _register_lua_renderer_api() {
    luasys_add_module_function(API_NAME, "render_world", _renderer_render_world);
    luasys_add_module_function(API_NAME, "set_debug", _renderer_set_debug);
}