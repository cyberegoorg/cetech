local ffi = require("ffi")
local api_system = require("cetech/api_system")

ffi.cdef [[
struct ct_transform {
    struct ct_world world;
    uint32_t idx;
};

struct ct_transform_a0 {
    int (*is_valid)(struct ct_transform transform);
    void (*transform)(struct ct_transform transform,
                      mat44f_t *parent);

    vec3f_t (*get_position)(struct ct_transform transform);

    quatf_t (*get_rotation)(struct ct_transform transform);

    vec3f_t (*get_scale)(struct ct_transform transform);

    mat44f_t *(*get_world_matrix)(struct ct_transform transform);

    void (*set_position)(struct ct_transform transform,
                         vec3f_t pos);

    void (*set_rotation)(struct ct_transform transform,
                         quatf_t rot);

    void (*set_scale)(struct ct_transform transform,
                      vec3f_t scale);

    int (*has)(struct ct_world world,
               struct ct_entity entity);

    struct ct_transform (*get)(struct ct_world world,
                               struct ct_entity entity);

    struct ct_transform (*create)(struct ct_world world,
                                  struct ct_entity entity,
                                  struct ct_entity parent,
                                  vec3f_t position,
                                  quatf_t rotation,
                                  vec3f_t scale);

    void (*link)(struct ct_world world,
                 struct ct_entity parent,
                 struct ct_entity child);
};
]]

local C = ffi.C
local api = api_system.load("ct_transform_a0")

--! dsadas
--! dasdasda
Transform = Transform or {}

--! Get transformation component
--! \param world lightuserdata Word
--! \param entity lightuserdata Entity
--! \return lightuserdata Transformation component
function Transform.get(world, entity)
    return api.get(world, entity)
end

--! Is transformation component valid?
--! \param transform lightuserdata Transformation component
--! \return boolean True if is valid.
function Transform.is_valid(transform)
    return api.is_valid(transform) > 0
end

--! Has entity transformation component?
--! \param world lightuserdata Word
--! \param entity lightuserdata Entity
--! \return boolean True if has.
function Transform.has(world, entity)
    return api.has(world, entity) > 0
end

--! Get position.
--! \param transform lightuserdata Transformation component.
--! \return cetech.Vec3f Position.
function Transform.get_position(transform)
    local p = api.get_position(transform)

    return cetech.Vec3f.make(p.x, p.y, p.z)
end

--! Get rotation.
--! \param transform lightuserdata Transformation component.
--! \return cetech.Quatf Rotation.
function Transform.get_rotation(transform)
    local r = api.get_rotation(transform)

    return cetech.Quatf.make(r.x, r.y, r.z, r.w)
end

--! Get scale.
--! \param transform lightuserdata Transformation component.
--! \return cetech.Vec3f Scale.
function Transform.get_scale(transform)
    local s = api.get_scale(transform)

    return cetech.Vec3f.make(s.x, s.y, s.z)
end

--! Get world matrix.
--! \param transform lightuserdata Transformation component.
--! \return cetech.Mat44f World matrix.
function Transform.get_world_matrix(transform)
    local m = api.get_world_matrix(transform)

    local r1 = cetech.Vec4f.make(m.x.x, m.x.y, m.x.z, m.x.w)
    local r2 = cetech.Vec4f.make(m.y.x, m.y.y, m.y.z, m.y.w)
    local r3 = cetech.Vec4f.make(m.z.x, m.z.y, m.z.z, m.z.w)
    local r4 = cetech.Vec4f.make(m.w.x, m.w.y, m.w.z, m.w.w)

    return cetech.Mat44f.make(r1, r2, r3, r4)
end

--! Set position.
--! \param transform lightuserdata Transformation component.
--! \param position cetech.Vec3f New position.
function Transform.set_position(transform, position)
    local p = ffi.new("struct vec3f_s", { { position.x, position.y, position.z } })

    api.set_position(transform, p)
end

--! Set rotation.
--! \param transform lightuserdata Transformation component.
--! \param rotation cetech.Quatf New rotation.
function Transform.set_rotation(transform, rotation)
    local r = ffi.new("struct vec4f_s", { { rotation.x, rotation.y, rotation.z, rotation.w } })

    api.set_rotation(transform, r)
end

--! Set scale.
--! \param transform lightuserdata Transformation component.
--! \param scale cetech.Vec3f New scale.
function Transform.set_scale(transform, scale)
    local s = ffi.new("struct vec3f_s", { { scale.x, scale.y, scale.z } })

    api.set_scale(transform, s)
end

return Transform