#include <cetech/celib/allocator.h>

#include <cetech/kernel/module.h>
#include <cetech/kernel/api_system.h>

#include <cetech/modules/entity.h>
#include <cetech/modules/luasys.h>

#include "cetech/modules/camera.h"
#include "cetech/modules/renderer.h"

#define API_NAME "Renderer"

CETECH_DECL_API(ct_renderer_api_v0)

static int _renderer_render_world(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    struct ct_camera c = {.idx = luasys_to_handler(l, 2)};

    ct_renderer_api_v0.render_world(w, c, 0);

    return 0;
}

static int _renderer_set_debug(lua_State *l) {
    int debug = luasys_to_bool(l, 1);

    ct_renderer_api_v0.set_debug(debug);
    return 0;
}

void _register_lua_renderer_api(struct ct_api_v0 *api) {
    CETECH_GET_API(api, ct_renderer_api_v0);

    luasys_add_module_function(API_NAME, "render_world",
                               _renderer_render_world);
    luasys_add_module_function(API_NAME, "set_debug", _renderer_set_debug);
}