local ffi = require("ffi")
local api_system  = require("cetech/api_system")
local hash_lib  = require("cetech/hashlib")

ffi.cdef[[
struct ct_material {
    uint32_t idx;
};

struct ct_material_a0 {
    struct ct_material (*resource_create)(uint64_t name);

    uint32_t (*get_texture_count)(struct ct_material material);

    void (*set_texture)(struct ct_material material,
                        const char *slot,
                        uint64_t texture);

    void (*set_vec4f)(struct ct_material material,
                      const char *slot,
                      vec4f_t v);

    void (*set_mat33f)(struct ct_material material,
                       const char *slot,
                       mat33f_t v);

    void (*set_mat44f)(struct ct_material material,
                       const char *slot,
                       mat44f_t v);

    void (*use)(struct ct_material material);

    void (*submit)(struct ct_material material);
};
]]

local C = ffi.C
local api = api_system.load("ct_material_a0")

Material = {}

function Material.set_texture(material, slot_name, texture_name)
    local id = hash_lib.id64_from_str(texture_name)
    return api.set_texture(material, slot_name, id)
end

function Material.set_vec4f(material, slot_name, value)
    local p = ffi.new("struct vec4f_s", {{ value.x, value.y, value.z }})
    return api.set_vec4f(material, slot_name, p)
end

return Material