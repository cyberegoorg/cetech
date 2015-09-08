#include <cstdio>

#include "platform/defines.h"

#include "common/log/log.h"
#include "common/memory/memory.h"
#include "common/container/array.h"

namespace cetech {
    namespace {
        struct Logger {
            Array < log::handler_t > handlers;
            Array < void* > handlers_data;

            Logger(Allocator & allocator) : handlers(allocator), handlers_data(allocator) {}
        };

        Logger* _logger;
    }

    namespace log_internal {
        void vlog(const log::ELogLevel level, const char* where, const char* format, va_list va) {
            char msg[4096]; //!< Final msg.
            vsnprintf(msg, 4096, format, va);

            const uint32_t handlers_count = array::size(_logger->handlers);
            for (uint32_t i = 0; i < handlers_count; ++i) {
                _logger->handlers[i](level, where, msg, _logger->handlers_data[i]);
            }
        }
    }

    namespace log {
        void init() {
            _logger = new Logger(memory_globals::default_allocator());//MAKE_NEW(memory_globals::default_allocator(), Logger, memory_globals::default_allocator());
        }

        void shutdown() {
            delete _logger;//MAKE_DELETE(memory_globals::default_allocator(), Logger, _logger);
            _logger = nullptr;
        }

        void register_handler(handler_t handler, void* data) {
            array::push_back(_logger->handlers, handler);
            array::push_back(_logger->handlers_data, data);
        }

        void info(const char* where, const char* format, ...) {
            va_list args;

            va_start(args, format);
            log_internal::vlog(LOG_INFO, where, format, args);
            va_end(args);
        }

        void warning(const char* where, const char* format, ...) {
            va_list args;

            va_start(args, format);
            log_internal::vlog(LOG_WARNING, where, format, args);
            va_end(args);
        }

        void error(const char* where, const char* format, ...) {
            va_list args;

            va_start(args, format);
            log_internal::vlog(LOG_ERROR, where, format, args);
            va_end(args);
        }

        void debug(const char* where, const char* format, ...) {
            #ifdef DEBUG
            va_list args;
            va_start(args, format);
            log_internal::vlog(LOG_DEBUG, where, format, args);
            va_end(args);
            #else
            //CE_UNUSED_PARAM(where);
            //CE_UNUSED_PARAM(format);
            #endif
        }
    }
}