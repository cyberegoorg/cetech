local ffi = require("ffi")

ffi.cdef[[
    void lua_log_info(const char* where, const char* format, ...);
    void lua_log_warning(const char* where, const char* format, ...);
    void lua_log_error(const char* where, const char* format, ...);
    void lua_log_debug(const char* where, const char* format, ...);
]]

Log = Log or {}
Log.info = ffi.C.lua_log_info
Log.warn = ffi.C.lua_log_warning
Log.error = ffi.C.lua_log_error
Log.debug = ffi.C.lua_log_debug

print = function (frmt, ...)
    Log.info("lua.print", "%s", string.format(frmt, ...))
end

return Log