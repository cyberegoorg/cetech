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
    namespace log_handlers {
        static void stdout_handler(const log::ELogLevel level, const char* where, const char* msg) {
            flockfile(stdout);
            fprintf(stdout, level_format[level], level_to_str[level], where, msg);
            funlockfile(stdout);
        }
    }
}