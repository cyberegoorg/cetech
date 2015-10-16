function Log.info(where, frmt, ...)
    Log._info(where, string.format(tostring(frmt), ...))
end

function Log.warn(where, frmt, ...)
    Log._warning(where, string.format(tostring(frmt), ...))
end

function Log.error(where, frmt, ...)
    Log._error(where, string.format(tostring(frmt), ...))
end

function Log.debug(where, frmt, ...)
    Log._debug(where, string.format(tostring(frmt), ...))
end

function print(frmt, ...)
    Log.info("lua.print", "%s", string.format(tostring(frmt), ...))
end

return Log