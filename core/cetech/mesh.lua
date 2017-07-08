local ffi = require("ffi")
local api_system  = require("cetech/api_system")
local hash_lib  = require("cetech/hashlib")

ffi.cdef[[
struct ct_mesh_renderer {
    struct ct_world world;
    uint32_t idx;
};

struct ct_mesh_renderer_a0 {

    int (*is_valid)(struct ct_mesh_renderer mesh);

    int (*has)(struct ct_world world,
               struct ct_entity entity);

    struct ct_mesh_renderer (*get)(struct ct_world world,
                                   struct ct_entity entity);

    struct ct_mesh_renderer (*create)(struct ct_world world,
                                      struct ct_entity entity,
                                      uint64_t scene,
                                      uint64_t mesh,
                                      uint64_t node,
                                      uint64_t material);

    struct ct_material (*get_material)(struct ct_mesh_renderer mesh);

    void (*set_material)(struct ct_mesh_renderer mesh,
                         uint64_t material);

    void (*render_all)(struct ct_world world);
};
]]

local C = ffi.C
local api = api_system.load("ct_mesh_renderer_a0")

Mesh = {}

function Mesh.get(world, entity)
    return api.get(world, entity)
end

function Mesh.is_valid(mesh)
    return api.is_valid(mesh) > 0
end

function Mesh.has(world, entity)
    return api.has(world, entity) > 0
end

function Mesh.get_material(mesh)
    return api.get_material(mesh)
end

function Mesh.set_material(mesh, material)
    local id = hash_lib.id64_from_str(material)
    return api.set_material(mesh, id)
end

return Mesh