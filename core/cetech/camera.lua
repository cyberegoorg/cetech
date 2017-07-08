local ffi = require("ffi")
local api_system = require("cetech/api_system")

ffi.cdef [[
struct ct_camera {
    struct ct_world world;
    uint32_t idx;
};

struct ct_camera_a0 {
    int (*has)(struct ct_world world,
               struct ct_entity entity);

    int (*is_valid)(struct ct_camera camera);

    void (*get_project_view)(struct ct_camera camera,
                             mat44f_t *proj,
                             mat44f_t *view);

    struct ct_camera (*get)(struct ct_world world,
                            struct ct_entity entity);

    struct ct_camera (*create)(struct ct_world world,
                               struct ct_entity entity,
                               float near,
                               float far,
                               float fov);
};
]]

local C = ffi.C
local api = api_system.load("ct_camera_a0")

Camera = {}

function Camera.get(world, entity)
    return api.get(world, entity)
end

function Camera.is_valid(camera)
    return api.is_valid(camera) > 0
end

function Camera.has(world, camera)
    return api.has(world, camera) > 0
end

return Camera