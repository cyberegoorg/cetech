local ffi = require("ffi")
local api_system  = require("cetech/api_system")
local hash_lib  = require("cetech/hashlib")

ffi.cdef[[
struct ct_package_a0 {
    void (*load)(uint64_t name);

    void (*unload)(uint64_t name);

    int (*is_loaded)(uint64_t name);

    void (*flush)(uint64_t name);
};
]]

local C = ffi.C
local api = api_system.load("ct_package_a0")

Package = {}

function Package.load(name)
    local id = hash_lib.id64_from_str(name)

    return api.load(id)
end

--! Load all resource that are in the package. If one is already loaded will not reload.
--!
--! !!! important
--!
--!     The function does not wait to finish loading.
--!     For querying whether the package is already loaded use method [**IsLoaded**](#Packageisloaded)
--!     or you can wait to load using method [**Flush**](#Packageflush)
function Package.load(name)
    local id = hash_lib.id64_from_str(name)

    return api.load(id)
end


function Package.is_loaded(name)
    local id = hash_lib.id64_from_str(name)

    return api.is_loaded(id) > 0
end


function Package.flush(name)
    local id = hash_lib.id64_from_str(name)

    return api.flush(id)
end


return Package