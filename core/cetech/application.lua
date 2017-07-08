local ffi = require("ffi")
local api_system  = require("cetech/api_system")

ffi.cdef[[
struct ct_app_a0 {
    void (*quit)();

    const char *(*platform)();

    const char *(*native_platform)();

    void *(*main_window)();
};
]]

local C = ffi.C
local api = api_system.load("ct_app_a0")

Application = {}

function Application.quit()
    return api.quit()
end

function Application.get_platform()
    return api.platform()
end

function Application.get_native_platform()
    return api.native_platform()
end


return Application