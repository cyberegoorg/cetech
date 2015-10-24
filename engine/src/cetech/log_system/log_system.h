#pragma once

#include <cstdio>
#include <cstdarg>
#include <ctime>

#include "celib/container/container_types.h"

namespace cetech {
    using namespace cetech;

    struct LogLevel {
        enum Enum {
            LOG_INFO,       //!< Information
            LOG_WARNING,    //!< Warning
            LOG_ERROR,      //<! Error
            LOG_DEBUG,      //<! Debug
        };
    };

    struct LogSystem {
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

        static void stdout_handler(const LogLevel::Enum level,
                                   const time_t time,
                                   const char* where,
                                   const char* msg,
                                   void* data);

        struct Implementation;
        Implementation* _impl;
    };

    namespace log_globals {
        void init();
        LogSystem& log();
        void shutdown();
    }
}