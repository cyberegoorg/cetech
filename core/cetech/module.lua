local ffi = require("ffi")
local api_system  = require("cetech/api_system")

ffi.cdef[[
typedef void (*ct_load_module_t)(struct ct_api_a0 *api, int reload);
typedef void (*ct_unload_module_t)(struct ct_api_a0 *api, int reload);
typedef void (*ct_initapi_module_t)(struct ct_api_a0 *api);

struct ct_module_a0 {
    void (*add_static)(ct_load_module_t load,
                       ct_unload_module_t unload,
                       ct_initapi_module_t initapi);
    void (*load)(const char *path);
    void (*load_dirs)(const char *path);
    void (*unload_all)();
    void (*reload)(const char *path);
    void (*reload_all)();
};
]]

local C = ffi.C
local api = api_system.load("ct_module_a0")

Module = {}

function Module.reload(path)
    return api.reload(path)
end

function Module.reload_all()
    return api.reload_all()
end

return Module