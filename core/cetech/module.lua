local ffi = require("ffi")
local api_system  = require("cetech/api_system")

ffi.cdef[[
struct ct_module_a0 {
    //! Reload module by path
    void (*module_reload)(const char *path);

    //! Reload all loaded modules
    void (*module_reload_all)();
};
]]

local C = ffi.C
local api = api_system.load("ct_module_a0")

Module = {}

function Module.reload(path)
    return api.module_reload(path)
end

function Module.reload_all()
    return api.module_reload_all()
end

return Module