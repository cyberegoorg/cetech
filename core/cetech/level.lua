local ffi = require("ffi")
local api_system  = require("cetech/api_system")
local hash_lib  = require("cetech/hashlib")

ffi.cdef[[
struct ct_level {
    uint32_t idx;
};

struct ct_level_a0 {

    struct ct_level (*load_level)(struct ct_world world,
                                  uint64_t name);

    void (*destroy)(struct ct_world world,
                    struct ct_level level);

    struct ct_entity (*entity_by_id)(struct ct_level level,
                                     uint64_t name);
                                     
    struct ct_entity (*entity)(struct ct_level level);
};
]]

local C = ffi.C
local api = api_system.load("ct_level_a0")

Level = {}

function Level.load_level(world, name)
    local id = hash_lib.id64_from_str(name)

    return api.load_level(world, id)
end

function Level.destroy(world, level)
    return api.destroy(world, level)
end

function Level.entity_by_id(level, name)
    local id = hash_lib.id64_from_str(name)

    return api.entity_by_id(level, id)
end

function Level.entity(level)
    return api.entity(level)
end

return Level