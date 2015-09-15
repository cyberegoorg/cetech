#pragma once

#include <cstdio>
#include <cstdarg>

#include "common/log/log.h"
#include "common/console_server/console_server.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/encodings.h"



#define LOG_FORMAT "[%u][%s][%s] %s"

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
        static void stdout_handler(const log::ELogLevel level, const time_t time, const char* where, const char* msg, void* data) {
            FILE* out;

            switch (level) {
            case log::LOG_ERROR:
                out = stdout;
                break;

            default:
                out = stderr;
                break;
            }
            
            flockfile(out);
            fprintf(out, level_format[level], time, level_to_str[level], where, msg);
            funlockfile(out);
        }

        static void file_handler(const log::ELogLevel level, const time_t time, const char* where, const char* msg, void* data) {
            FILE* out = (FILE*)(data);

            flockfile(out);
            fprintf(out, LOG_FORMAT "\n", time, level_to_str[level], where, msg);
            fflush(out);
            funlockfile(out);   
        }

        static void console_server_handler(const log::ELogLevel level, const time_t time, const char* where, const char* msg, void* data) {
            if( !console_server_globals::has_clients() ) {
                return;
            }

            rapidjson::Document json_data;
            json_data.SetObject();
            
            json_data.AddMember("time", rapidjson::Value(time), json_data.GetAllocator());
            json_data.AddMember("level", rapidjson::Value(level_to_str[level], 1), json_data.GetAllocator());
            json_data.AddMember("where", rapidjson::Value(where, strlen(where)), json_data.GetAllocator());
            json_data.AddMember("msg", rapidjson::Value(msg, strlen(msg)), json_data.GetAllocator());

            console_server_globals::send_message("log", json_data);

        }
    }
}