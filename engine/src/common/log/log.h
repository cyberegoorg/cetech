#pragma once

#include <cstdio>
#include <cstdarg>

namespace cetech {
    namespace log {
        enum ELogLevel {
            LOG_INFO,       //!< Information
            LOG_WARNING,    //!< Warning
            LOG_ERROR,      //<! Error
            LOG_DEBUG,      //<! Debug
        };
        typedef void (* handler_t)(const log::ELogLevel, const char*, const char*, void*);

        void init();
        void shutdown();

        void register_handler(handler_t handler, void* data = nullptr);

        void info(const char* where, const char* format, ...);
        void warning(const char* where, const char* format, ...);
        void error(const char* where, const char* format, ...);
        void debug(const char* where, const char* format, ...);
    }
}