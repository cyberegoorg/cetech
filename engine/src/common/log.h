#pragma once

#include <cstdio>
#include <cstdarg>

#define LOG_FORMAT "[%s][%s] %s"

#define COLOR_RED  "\x1B[31m"
#define COLOR_GREEN  "\x1B[32m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_BLUE  "\x1B[34m"
#define COLOR_RESET "\033[0m"

#ifdef CETECH_COLORED_LOG
  #define COLORED_TEXT(color, text) color text COLOR_RESET
#else
  #define COLORED_TEXT(color, text) text
#endif

static const char* level_to_str[] = { "I", "W", "E", "D" };
static const char* level_format[] = {
    COLORED_TEXT(COLOR_BLUE, LOG_FORMAT) "\n",          /* INFO    */
    COLORED_TEXT(COLOR_YELLOW, LOG_FORMAT) "\n",        /* WARNING */
    COLORED_TEXT(COLOR_RED, LOG_FORMAT) "\n",           /* ERROR   */
    COLORED_TEXT(COLOR_GREEN, LOG_FORMAT) "\n"          /* DEBUG   */
};

namespace cetech {
    namespace log {
        enum ELogLevel {
            LOG_INFO,       //!< Information
            LOG_WARNING,    //!< Warning
            LOG_ERROR,      //<! Error
            LOG_DEBUG,      //<! Debug
        };

        static void info(const char* where, const char* format, ...);
        static void warning(const char* where, const char* format, ...);
        static void error(const char* where, const char* format, ...);
        static void debug(const char* where, const char* format, ...);
    }

    namespace log_internal {
        CE_INLINE void log_vprntf(const log::ELogLevel level, const char* where, const char* format, va_list va) {
            char msg[1024];         //!< Final msg.

            vsnprintf(msg, 1024, format, va);
            flockfile(stdout);
            fprintf(stdout, level_format[level], level_to_str[level], where, msg);
            funlockfile(stdout);
        }
    }

    namespace log {
        void info(const char* where, const char* format, ...) {
            va_list args;

            va_start(args, format);
            log_internal::log_vprntf(LOG_INFO, where, format, args);
            va_end(args);
        }

        void warning(const char* where, const char* format, ...) {
            va_list args;

            va_start(args, format);
            log_internal::log_vprntf(LOG_WARNING, where, format, args);
            va_end(args);
        }

        void error(const char* where, const char* format, ...) {
            va_list args;

            va_start(args, format);
            log_internal::log_vprntf(LOG_ERROR, where, format, args);
            va_end(args);
        }

        void debug(const char* where, const char* format, ...) {
            #ifdef DEBUG
            va_list args;
            va_start(args, format);
            log_internal::log_vprntf(LOG_DEBUG, where, format, args);
            va_end(args);
            #else
            //CE_UNUSED_PARAM(where);
            //CE_UNUSED_PARAM(format);
            #endif
        }
    }
}