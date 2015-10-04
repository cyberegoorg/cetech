#include "celib/log/log.h"

#define EXPORT extern "C" __attribute__ ((dllexport))

EXPORT void lua_log_info(const char* where, const char* format, ...) {
    va_list args;

    va_start(args, format);
    cetech::log::info(where, format, args);
    va_end(args);
}

EXPORT void lua_log_warning(const char* where, const char* format, ...) {
    va_list args;

    va_start(args, format);
    cetech::log::warning(where, format, args);
    va_end(args);
}

EXPORT void lua_log_error(const char* where, const char* format, ...) {
    va_list args;

    va_start(args, format);
    cetech::log::error(where, format, args);
    va_end(args);
}

EXPORT void lua_log_debug(const char* where, const char* format, ...) {
    va_list args;

    va_start(args, format);
    cetech::log::debug(where, format, args);
    va_end(args);
}