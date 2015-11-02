#include <cstdio>
#include <cstdarg>
#include <ctime>

#include "cetech/log_system/log_system.h"
#include "celib/macros.h"
#include "cetech/develop/console_server.h"
#include "cetech/application/application.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/encodings.h"


#define LOG_FORMAT_NO_TIME "[%s][%d][%s] %s"
#define LOG_FORMAT  "[%s]" LOG_FORMAT_NO_TIME

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
    COLORED_TEXT(COLOR_BLUE, LOG_FORMAT_NO_TIME) "\n",          /* INFO    */
    COLORED_TEXT(COLOR_YELLOW, LOG_FORMAT_NO_TIME) "\n",        /* WARNING */
    COLORED_TEXT(COLOR_RED, LOG_FORMAT_NO_TIME) "\n",           /* ERROR   */
    COLORED_TEXT(COLOR_GREEN, LOG_FORMAT_NO_TIME) "\n"          /* DEBUG   */
};

namespace cetech {
    namespace {
        static char* time_to_utc_str(std::tm* gmtm) {
            char* time_str = std::asctime(gmtm);
            time_str[strlen(time_str) - 1] = '\0';
            return time_str;
        }
    };

    namespace log_handlers {
        void stdout_handler(const log::LogLevel::Enum level,
                            const time_t time,
                            const uint32_t worker_id,
                            const char* where,
                            const char* msg,
                            void* data) {
            CE_UNUSED(time);
            CE_UNUSED(data);

            FILE* out;

            switch (level) {
            case log::LogLevel::LOG_ERROR:
                out = stderr;
                break;

            default:
                out = stdout;
                break;
            }

            flockfile(out);
            fprintf(out, level_format[level], level_to_str[level], worker_id, where, msg);
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
            fprintf(out, LOG_FORMAT "\n", time_str, level_to_str[level], worker_id, where, msg);
            fflush(out);
            funlockfile(out);
        }
    }
}