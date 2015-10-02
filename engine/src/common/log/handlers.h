#pragma once

#include <cstdio>
#include <cstdarg>
#include <ctime>

#include "common/log/log.h"
#include "develop/console_server.h"
#include "application/application.h"
#include <os/os.h>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/encodings.h"


#define LOG_FORMAT_NO_TIME "[%s][%s] %s"
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
    namespace log_handlers_internal {
        static char* time_to_utc_str(std::tm* gmtm) {
            char* time_str = std::asctime(gmtm);
            time_str[strlen(time_str) - 1] = '\0';
            return time_str;
        }
    };

    namespace log_handlers {
        static void stdout_handler(const log::ELogLevel level,
                                   const time_t time,
                                   const char* where,
                                   const char* msg,
                                   void* data) {
            FILE* out;

            switch (level) {
            case log::LOG_ERROR:
                out = stdout;
                break;

            default:
                out = stderr;
                break;
            }

            std::tm* gmtm = std::gmtime(&time);
            char* time_str = log_handlers_internal::time_to_utc_str(gmtm);

            flockfile(out);
            fprintf(out, level_format[level], level_to_str[level], where, msg);
            funlockfile(out);
        }

        static void file_handler(const log::ELogLevel level,
                                 const time_t time,
                                 const char* where,
                                 const char* msg,
                                 void* data) {
            FILE* out = (FILE*)(data);

            std::tm* gmtm = std::gmtime(&time);
            char* time_str = log_handlers_internal::time_to_utc_str(gmtm);

            flockfile(out);
            fprintf(out, LOG_FORMAT "\n", time_str, level_to_str[level], where, msg);
            fflush(out);
            funlockfile(out);
        }

        static void console_server_handler(const log::ELogLevel level,
                                           const time_t time,
                                           const char* where,
                                           const char* msg,
                                           void* data) {

            ConsoleServer& cs = application_globals::app().console_server();

            if (!cs.has_clients()) {
                return;
            }

            std::tm* gmtm = std::gmtime(&time);
            char* time_str = log_handlers_internal::time_to_utc_str(gmtm);

            rapidjson::Document json_data;
            json_data.SetObject();

            json_data.AddMember("type", "log", json_data.GetAllocator());

            json_data.AddMember("time", rapidjson::Value(time), json_data.GetAllocator());
            json_data.AddMember("level", rapidjson::Value(level_to_str[level], 1), json_data.GetAllocator());
            json_data.AddMember("where", rapidjson::Value(where, strlen(where)), json_data.GetAllocator());
            json_data.AddMember("msg", rapidjson::Value(msg, strlen(msg)), json_data.GetAllocator());

            cs.send_json_document(json_data);
        }
    }
}