local ffi = require("ffi")
local api_system  = require("cetech/api_system")
local hash_lib  = require("cetech/hashlib")

ffi.cdef[[
struct ct_blob;

typedef struct {
    void* v;
} yaml_node_t;

typedef int (*ct_component_compiler_t)(yaml_node_t body,
                                       struct ct_blob *data);


enum ct_property_type {
    PROPERTY_INVALID = 0,
    PROPERTY_BOOL,
    PROPERTY_FLOAT,
    PROPERTY_STRING,
    PROPERTY_STRINGID64,
    PROPERTY_VEC3,
    PROPERTY_QUATF,
};

struct ct_property_value {
    enum ct_property_type type;

    union {
        int b;
        float f;
        const char *str;
        float vec3f[3];
        float quatf[4];
        uint64_t strid_64;
    } value;
};

static struct ct_component_clb {
    void (*destroyer)(struct ct_world world,
                      struct ct_entity *ents,
                      size_t ent_count);

    void (*spawner)(struct ct_world world,
                    struct ct_entity *ents,
                    uint32_t *cent,
                    uint32_t *ents_parent,
                    size_t ent_count,
                    void *data);

    void (*set_property)(struct ct_world world,
                         struct ct_entity entity,
                         uint64_t key,
                         struct ct_property_value value);

    struct ct_property_value (*get_property)(struct ct_world world,
                                             struct ct_entity entity,
                                             uint64_t key);
    ct_world_callbacks_t world_clb;
};


struct ct_component_a0 {
    void (*register_compiler)(uint64_t type,
                              ct_component_compiler_t compiler,
                              uint32_t spawn_order);

    int (*compile)(uint64_t type,
                   yaml_node_t body,
                   struct ct_blob *data);

    uint32_t (*spawn_order)(uint64_t type);

    void (*register_type)(uint64_t type,
                          struct ct_component_clb clb);

    void (*spawn)(struct ct_world world,
                  uint64_t type,
                  struct ct_entity *ent_ids,
                  uint32_t *cent,
                  uint32_t *ents_parent,
                  uint32_t ent_count,
                  void *data);

    void (*destroy)(struct ct_world world,
                    struct ct_entity *ent,
                    uint32_t count);

    void (*set_property)(uint64_t type,
                         struct ct_world world,
                         struct ct_entity entity,
                         uint64_t key,
                         struct ct_property_value value);

    struct ct_property_value (*get_property)(uint64_t type,
                                             struct ct_world world,
                                             struct ct_entity entity,
                                             uint64_t key);
};
]]

local C = ffi.C

local api = api_system.load("ct_component_a0")

Component = {}

function Component.set_property(world, entity, component_type, key, value)
    local component_type = hash_lib.id64_from_str(component_type)
    local key_id = hash_lib.id64_from_str(key)

    local value_type = type(value)
    local prop_value

    if value_type == 'number' then
        prop_value = ffi.new("struct ct_property_value", { C.PROPERTY_FLOAT, { f=value }})
    elseif value_type == 'string' then
        prop_value = ffi.new("struct ct_property_value", { C.PROPERTY_STRING, { str=value }})
    elseif value_type == 'boolean' then
        prop_value = ffi.new("struct ct_property_value", { C.PROPERTY_BOOL, { b=value }})
    else
        if cetech.Vec3f.is(value) then
            prop_value = ffi.new("struct ct_property_value", { C.PROPERTY_VEC3, { vec3f=ffi.cast("float*", value)[0] }})
        elseif cetech.Quatf.is(value) then
            prop_value = ffi.new("struct ct_property_value", { C.PROPERTY_QUATF, { quatf=ffi.cast("float*", value)[0] }})
        end
    end

    return api.set_property(component_type, world, entity, key_id, prop_value)
end

function Component.get_property(world, entity, component_type, key)
    local component_type = hash_lib.id64_from_str(component_type)
    local key_id = hash_lib.id64_from_str(key)

    local prop_value = api.get_property(component_type, world, entity, key_id)

    if prop_value.type == C.PROPERTY_FLOAT then
        return prop_value.value.f

    elseif prop_value.type == C.PROPERTY_STRING then
        return prop_value.value.str

    elseif prop_value.type == C.PROPERTY_BOOL then
        return prop_value.value.b

    elseif prop_value.type == C.PROPERTY_VEC3 then
        local v = prop_value.value.vec3f
        return cetech.Vec3f.make(v[0], v[1], v[2])

    elseif prop_value.type == C.PROPERTY_QUATF then
        local q = prop_value.value.quatf
        return cetech.Quatf.make(q[0], q[1], q[2], q[3])
    end
end

return Component
