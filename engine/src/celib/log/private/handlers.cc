#include <cstdio>
#include <cstdarg>
#include <ctime>

#include "celib/macros.h"
#include "celib/defines.h"
#include "celib/stacktrace/stacktrace.h"

#include "celib/log/log.h"
#include "cetech/develop/console_server.h"
#include "cetech/application/application.h"


#define LOG_FORMAT "---\n" \
    "level: %s\n" \
    "where: %s\n" \
    "time: %s\n" \
    "worker: %d\n" \
    "msg: |\n  %s\n"

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

static const char* level_to_str[] = { "info", "warning", "error", "debug" };
static const char* level_format[] = {
    COLORED_TEXT(COLOR_BLUE, LOG_FORMAT),   /* INFO    */
    COLORED_TEXT(COLOR_YELLOW, LOG_FORMAT), /* WARNING */
    COLORED_TEXT(COLOR_RED, LOG_FORMAT),    /* ERROR   */
    COLORED_TEXT(COLOR_GREEN, LOG_FORMAT)   /* DEBUG   */
};

static const char* nocolor_level_format[] = {
    LOG_FORMAT, /* INFO    */
    LOG_FORMAT, /* WARNING */
    LOG_FORMAT, /* ERROR   */
    LOG_FORMAT  /* DEBUG   */
};

namespace cetech {
    namespace {
        static char* time_to_utc_str(std::tm* gmtm) {
            char* time_str = std::asctime(gmtm);
            time_str[strlen(time_str) - 1] = '\0';
            return time_str;
        }
    };

    namespace log {
        void stdout_handler(const log::LogLevel::Enum level,
                            const time_t time,
                            const uint32_t worker_id,
                            const char* where,
                            const char* msg,
                            void* data) {
            CE_UNUSED(data);

            FILE* out;

            std::tm* gmtm = std::gmtime(&time);
            char* time_str = time_to_utc_str(gmtm);

            switch (level) {
            case log::LogLevel::ERROR:
                out = stderr;
                break;

            default:
                out = stdout;
                break;
            }

            flockfile(out);
            fprintf(out, level_format[level], level_to_str[level], where, time_str, worker_id, msg);
            funlockfile(out);
        }

        void file_handler(const log::LogLevel::Enum level,
                          const time_t time,
                          const uint32_t worker_id,
                          const char* where,
                          const char* msg,
                          void* data) {
            FILE* out = (FILE*)(data);

            std::tm* gmtm = std::gmtime(&time);
            char* time_str = time_to_utc_str(gmtm);


            flockfile(out);
            fprintf(out, nocolor_level_format[level], level_to_str[level], where, time_str, worker_id, msg);
            fflush(out);
            funlockfile(out);
        }
    }
}
