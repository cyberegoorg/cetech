#include <cstdio>
#include <ctime>

#include "celib/defines.h"

#include "cetech/log_system/log_system.h"
#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"

namespace cetech {
    struct LogSystem::Implementation {
        Implementation(Allocator & allocator) : handlers(allocator), handlers_data(allocator) {}
        ~Implementation() {};

        void vlog(const LogLevel::Enum level, const char* where, const char* format, va_list va) {
            char msg[4096]; //!< Final msg.
            vsnprintf(msg, 4096, format, va);

            time_t tm = std::time(NULL);
            const uint32_t handlers_count = array::size(handlers);
            for (uint32_t i = 0; i < handlers_count; ++i) {
                if (handlers[i] == nullptr) {
                    continue;
                }

                handlers[i](level, tm, where, msg, handlers_data[i]);
            }
        }

        void register_handler(handler_t handler, void* data) {
            array::push_back(handlers, handler);
            array::push_back(handlers_data, data);
        }

        void unregister_handler(handler_t handler) {
            for (uint32_t i = 0; i < array::size(handlers); ++i) {
                if (handlers[i] != handler) {
                    continue;
                }

                handlers[i] = nullptr;
            }
        }

        Array < handler_t > handlers;
        Array < void* > handlers_data;
    };
}

namespace cetech {
    void LogSystem::register_handler(handler_t handler, void* data) {
        _impl->register_handler(handler, data);
    }

    void LogSystem::unregister_handler(handler_t handler) {
        _impl->unregister_handler(handler);
    }

    void LogSystem::info(const char* where, const char* format, va_list va) {
        _impl->vlog(LogLevel::LOG_INFO, where, format, va);
    }

    void LogSystem::info(const char* where, const char* format, ...) {
        va_list args;

        va_start(args, format);
        info(where, format, args);
        va_end(args);
    }

    void LogSystem::warning(const char* where, const char* format, va_list va) {
        _impl->vlog(LogLevel::LOG_WARNING, where, format, va);
    }

    void LogSystem::warning(const char* where, const char* format, ...) {
        va_list args;

        va_start(args, format);
        warning(where, format, args);
        va_end(args);
    }


    void LogSystem::error(const char* where, const char* format, va_list va) {
        _impl->vlog(LogLevel::LOG_ERROR, where, format, va);
    }

    void LogSystem::error(const char* where, const char* format, ...) {
        va_list args;

        va_start(args, format);
        error(where, format, args);
        va_end(args);
    }

    void LogSystem::debug(const char* where, const char* format, va_list va) {
        #ifdef DEBUG
        _impl->vlog(LogLevel::LOG_DEBUG, where, format, va);
        #else
        //CE_UNUSED_PARAM(where);
        //CE_UNUSED_PARAM(format);
        #endif
    }


    void LogSystem::debug(const char* where, const char* format, ...) {
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

namespace {
    using namespace cetech;
    struct LogGlobals {
        static const int LOGGER_MEMORY = sizeof(LogSystem) + sizeof(LogSystem::Implementation);
        char buffer[LOGGER_MEMORY];

        LogSystem* logger;
        LogSystem::Implementation* logger_impl;

        LogGlobals() : logger(0), logger_impl() {}
    };

    LogGlobals _log_globals;
}

namespace cetech {
    namespace log_globals {
        void init() {
            char* p = _log_globals.buffer;

            _log_globals.logger = new(p) LogSystem();
            _log_globals.logger_impl = new( p + sizeof(LogSystem)) LogSystem::Implementation(
                memory_globals::default_allocator());

            _log_globals.logger->_impl = _log_globals.logger_impl;
        }

        LogSystem& log() {
            return *(_log_globals.logger);
        }

        void shutdown() {
            //_log_globals.logger->~Logger();
            _log_globals.logger_impl->~Implementation();
            _log_globals = LogGlobals();

        }
    }
}