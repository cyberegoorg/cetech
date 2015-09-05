#pragma once

#include <cstdio>
#include <cstdarg>

#include "platform/defines.h"


namespace cetech {
    namespace log {
        enum ELogLevel {
            LOG_INFO,       //!< Information
            LOG_WARNING,    //!< Warning
            LOG_ERROR,      //<! Error
            LOG_DEBUG,      //<! Debug
        };

        void info(const char* where, const char* format, ...);
        void warning(const char* where, const char* format, ...);
        void error(const char* where, const char* format, ...);
        void debug(const char* where, const char* format, ...);
    }
}