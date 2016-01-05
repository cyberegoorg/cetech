#include <cstdio>
#include <ctime>

#include "celib/defines.h"

#include "cetech/log/log.h"
#include <cetech/task_manager/task_manager.h>
#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"


namespace cetech {
    namespace {
        using namespace log;
        struct LogData {
            enum {
                MAX_HANDLERS = 32,
            };

            handler_t handlers[MAX_HANDLERS];
            void* handlers_data[MAX_HANDLERS];
            log_globals::get_worker_id_clb_t get_worker_id_clb;
            uint8_t handlers_count;

            LogData(const log_globals::get_worker_id_clb_t get_worker_id_clb) : get_worker_id_clb(get_worker_id_clb) {
                memset(this, 0, sizeof(LogData));
            }
        };

        struct Globals {
            static const int MEMORY = sizeof(LogData);
            char buffer[MEMORY];

            LogData* data;

            Globals() : buffer {
                0
            }, data(0) {}
        } _globals;

        void vlog(const LogLevel::Enum level,
                  const char* where,
                  const char* format,
                  va_list va) {
            CE_ASSERT("log", _globals.data != nullptr);
            LogData& data = *_globals.data;

            char msg[4096];     //!< Final msg.
            vsnprintf(msg, 4096, format, va);

            time_t tm = std::time(NULL);

            for (uint32_t i = 0; i < LogData::MAX_HANDLERS; ++i) {
                if (data.handlers[i] == nullptr) {
                    continue;
                }

                data.handlers[i](level, tm, task_manager::get_worker_id(), where, msg, data.handlers_data[i]);
            }
        }
    }

    namespace log {
        void register_handler(handler_t handler,
                              void* data) {
            CE_ASSERT("log", _globals.data != nullptr);
            LogData& log_data = *_globals.data;

            uint8_t count = log_data.handlers_count;
            ++log_data.handlers_count;

            log_data.handlers[count] = handler;
            log_data.handlers_data[count] = data;
        }

        void unregister_handler(handler_t handler) {
            CE_ASSERT("log", _globals.data != nullptr);
            LogData& log_data = *_globals.data;


            uint8_t count = log_data.handlers_count;
            for (uint32_t i = 0; i < count; ++i) {
                if (log_data.handlers[i] != handler) {
                    continue;
                }

                log_data.handlers[i] = nullptr;
            }
        }


        void info(const char* where,
                  const char* format,
                  va_list va) {
            vlog(LogLevel::INFO, where, format, va);
        }

        void info(const char* where,
                  const char* format,
                  ...) {
            va_list args;

            va_start(args, format);
            info(where, format, args);
            va_end(args);
        }

        void warning(const char* where,
                     const char* format,
                     va_list va) {
            vlog(LogLevel::WARNING, where, format, va);
        }

        void warning(const char* where,
                     const char* format,
                     ...) {
            va_list args;

            va_start(args, format);
            warning(where, format, args);
            va_end(args);
        }


        void error(const char* where,
                   const char* format,
                   va_list va) {
            vlog(LogLevel::ERROR, where, format, va);
        }

        void error(const char* where,
                   const char* format,
                   ...) {
            va_list args;

            va_start(args, format);
            error(where, format, args);
            va_end(args);
        }

        void debug(const char* where,
                   const char* format,
                   va_list va) {
        #ifdef DEBUG
            vlog(LogLevel::DBG, where, format, va);
        #else
            //CE_UNUSED_PARAM(where);
            //CE_UNUSED_PARAM(format);
        #endif
        }


        void debug(const char* where,
                   const char* format,
                   ...) {
        #ifdef DEBUG
            va_list args;
            va_start(args, format);
            debug(where, format, args);
            va_end(args);
        #else
            //CE_UNUSED_PARAM(where);
            //CE_UNUSED_PARAM(format);
        #endif
        }
    }

    namespace log_globals {
        void init(const log_globals::get_worker_id_clb_t get_worker_id_clb) {
            char* p = _globals.buffer;

            _globals.data = new(p) LogData(get_worker_id_clb);

            log::info("log_globals", "Initialized");
        }

        void shutdown() {
            log::info("log_globals", "Shutdown");

            _globals.data->~LogData();
            _globals = Globals();

        }
    }
}
