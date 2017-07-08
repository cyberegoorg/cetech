local ffi = require("ffi")
local api_system  = require("cetech/api_system")

ffi.cdef[[
typedef int viewport_t;

typedef struct {
    uint64_t a;
} ct_window_t;


struct ct_renderer_a0 {
    void (*create)(ct_window_t *window);

    void (*set_debug)(int debug);

    vec2f_t (*get_size)();

    void (*render_world)(struct ct_world world,
                         struct ct_camera camera,
                         viewport_t viewport);
};
]]

local C = ffi.C
local api = api_system.load("ct_renderer_a0")

Renderer = {}

function Renderer.render_world(world, camera, viewport)
    return api.render_world(world, camera, 0)
end

function Renderer.set_debug(debug)
    return api.set_debug(debug)
end


return Renderer