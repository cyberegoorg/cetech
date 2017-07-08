local ffi = require("ffi")
local api_system  = require("cetech/api_system")

ffi.cdef[[
struct ct_hash_a0 {

    //! 64bit murmur 2 hash
    //! \param key Key
    //! \param seed Seed
    //! \return Hashed value
    uint64_t (*hash_murmur2_64)(const void *key,
                                uint64_t len,
                                uint64_t seed);

    uint64_t (*id64_from_str)(const char *str);
};
]]

local C = ffi.C
local api = api_system.load("ct_hash_a0")

HashLib = {}

function HashLib.hash_murmur2_64(key, len, seed)
    return api.hash_murmur2_64(key, len, seed)
end

function HashLib.id64_from_str(str)
    return api.id64_from_str(str)
end

return HashLib