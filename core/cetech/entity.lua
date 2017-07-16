local ffi = require("ffi")

local api_system  = require("cetech/api_system")
local hash_lib  = require("cetech/hashlib")

ffi.cdef[[
struct ct_entity {
    uint32_t h;
};

struct ct_entity_a0 {
    struct ct_entity (*create)();

    void (*destroy)(struct ct_world world,
                    struct ct_entity *entity,
                    uint32_t count);

    int (*alive)(struct ct_entity entity);


    void (*spawn_from_resource)(struct ct_world world,
                                void *resource,
                                struct ct_entity **entities,
                                uint32_t *entities_count);

    struct ct_entity (*spawn)(struct ct_world world,
                              uint64_t name);
};
]]

local C = ffi.C
local api = api_system.load("ct_entity_a0")

Entity = {}

function Entity.destroy(entity)
    return api.destroy(entity)
end

function Entity.spawn(world, name)
    local id = hash_lib.id64_from_str(name)
    return api.spawn(world, id)
end

return Entity