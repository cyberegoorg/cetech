local ffi = require("ffi")
local api_system  = require("cetech/api_system")
local hash_lib  = require("cetech/hashlib")

ffi.cdef[[
typedef struct {
    uint64_t name;
    uint64_t type;
    uint8_t input_count;
    uint8_t output_count;
    uint64_t input[8];
    uint64_t output[8];
} layer_entry_t;

struct ct_texture {
    uint16_t idx;
};

struct ct_viewport {
    uint32_t idx;
};

typedef void (*ct_render_on_render)();
typedef void(*ct_renderer_on_pass_t)(layer_entry_t entry, uint8_t viewid, struct ct_world world, struct ct_camera camera);

struct ct_renderer_a0 {
    void (*create)();

    void (*set_debug)(int debug);

    void (*get_size)(uint32_t *width,
                     uint32_t *height);


    struct ct_texture (*get_global_resource)(uint64_t name);

    void (*render_world)(struct ct_world world,
                         struct ct_camera camera,
                          struct ct_viewport viewport);

    void (*register_layer_pass)(uint64_t  type, ct_renderer_on_pass_t on_pass);

    void (*register_on_render)(ct_render_on_render on_render);
    void (*unregister_on_render)(ct_render_on_render on_render);
     struct ct_viewport (*create_viewport)(uint64_t name, float width, float height);
};
]]

local C = ffi.C
local api = api_system.load("ct_renderer_a0")

Renderer = {}

function Renderer.render_world(world, camera, viewport)
    return api.render_world(world, camera, viewport)
end

function Renderer.set_debug(debug)
    return api.set_debug(debug)
end

function Renderer.create_viewport(name)
    local id = hash_lib.id64_from_str(name)

    return api.create_viewport(id, 0, 0)
end

return Renderer