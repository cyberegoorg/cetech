#pragma once

#include <cstdio>
#include <cstdarg>
#include <ctime>

#include "cetech/log_system/log_system.h"

namespace cetech {
    namespace log_handlers {
        void stdout_handler(const log::LogLevel::Enum level,
                            const time_t time,
                            const uint32_t worker_id,
                            const char* where,
                            const char* msg,
                            void* data);

        void file_handler(const log::LogLevel::Enum level,
                          const time_t time,
                          const uint32_t worker_id,
                          const char* where,
                          const char* msg,
                          void* data);
    }
}