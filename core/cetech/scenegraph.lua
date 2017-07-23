local ffi = require("ffi")
local api_system = require("cetech/api_system")
local hash_lib = require("cetech/hashlib")

ffi.cdef [[
struct ct_scene_node {
    struct ct_world world;
    uint32_t idx;
};

struct ct_scenegprah_a0 {

    int (*is_valid)(struct ct_scene_node node);

    void (*get_position)(struct ct_scene_node node, float *value);

    void (*get_rotation)(struct ct_scene_node node, float *value);

    void (*get_scale)(struct ct_scene_node node, float *value);

    void *(*get_world_matrix)(struct ct_scene_node node, float *value);

    void (*set_position)(struct ct_scene_node node,
                         float* pos);

    void (*set_rotation)(struct ct_scene_node node,
                         float* rot);

    void (*set_scale)(struct ct_scene_node node,
                      float* scale);

    int (*has)(struct ct_world world,
               struct ct_entity entity);

    struct ct_scene_node (*get_root)(struct ct_world world,
                                     struct ct_entity entity);

    struct ct_scene_node (*create)(struct ct_world world,
                                   struct ct_entity entity,
                                   uint64_t *names,
                                   uint32_t *parent,
                                   float *pose,
                                   uint32_t count);

    void (*link)(struct ct_scene_node parent,
                 struct ct_scene_node child);

    struct ct_scene_node (*node_by_name)(struct ct_world world,
                                         struct ct_entity entity,
                                         uint64_t name);
};

]]

local C = ffi.C
local api = api_system.load("ct_scenegprah_a0")

SceneGraph = {}

function SceneGraph.get(world, entity)
    return api.get(world, entity)
end

function SceneGraph.is_valid(scene_node)
    return api.is_valid(scene_node) > 0
end

function SceneGraph.has(world, entity)
    return api.has(world, entity) > 0
end

function SceneGraph.get_position(scene_node)
    local p = ffi.new("float[3]")

    api.get_position(scene_node, p)

    return cetech.Vec3f.make(p[0], p[1], p[2])
end

function SceneGraph.get_rotation(scene_node)
    local p = ffi.new("float[4]")

    api.get_rotation(scene_node, p)

    return cetech.Quatf.make(p[0], p[1], p[2], p[3])
end

function SceneGraph.get_scale(scene_node)
    local p = ffi.new("float[3]")

    api.get_scale(scene_node, p)

    return cetech.Vec3f.make(p[0], p[1], p[2])
end

function SceneGraph.set_position(scene_node, position)
    local p = ffi.new("float[3]", { position.x, position.y, position.z })

    api.set_position(scene_node, p)
end

function SceneGraph.set_rotation(scene_node, rotation)
    local r = ffi.new("float[4]", { rotation.x, rotation.y, rotation.z, rotation.w })

    api.set_rotation(scene_node, r)
end

function SceneGraph.set_scale(scene_node, scale)
    local s = ffi.new("float[3]", { scale.x, scale.y, scale.z })

    api.set_scale(scene_node, s)
end

function SceneGraph.node_by_name(world, entity, name)
    local id = hash_lib.id64_from_str(name)

    return api.node_by_name(world, entity, id)
end

return SceneGraph