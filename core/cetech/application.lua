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

--! Module contain functions for Application.
Application = {}

--! Quit application
function Application.quit()
    return api.quit()
end

--! Get platform
--! \return string Platform
function Application.get_platform()
    return api.platform()
end

--! Get native platform
--! \return string native platform
function Application.get_native_platform()
    return api.native_platform()
end

return Application