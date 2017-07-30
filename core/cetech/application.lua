local ffi = require("ffi")
local api_system  = require("cetech/api_system")

ffi.cdef[[
struct ct_app_a0 {
    void (*quit)();
    void (*start)();
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

return Application