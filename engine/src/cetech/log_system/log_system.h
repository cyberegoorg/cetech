#pragma once

#include <cstdio>
#include <cstdarg>
#include <ctime>

#include "celib/container/container_types.h"

namespace cetech {
    namespace log {
        struct LogLevel {
            enum Enum {
                LOG_INFO,       //!< Information
                LOG_WARNING,    //!< Warning
                LOG_ERROR,      //<! Error
                LOG_DEBUG,      //<! Debug
            };
        };

        typedef void (* handler_t)(const LogLevel::Enum, const time_t, const char*, const char*, void*);

        void register_handler(handler_t handler, void* data = nullptr);
        void unregister_handler(handler_t handler);

        void info(const char* where, const char* format, va_list va);
        void info(const char* where, const char* format, ...);

        void warning(const char* where, const char* format, va_list va);
        void warning(const char* where, const char* format, ...);

        void error(const char* where, const char* format, va_list va);
        void error(const char* where, const char* format, ...);

        void debug(const char* where, const char* format, va_list va);
        void debug(const char* where, const char* format, ...);

    }

    namespace log_globals {
        void init();
        void shutdown();
    }
}