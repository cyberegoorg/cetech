#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include <cstdio>
#include <cstdarg>
#include <ctime>

#include "celib/container/types.h"

/*******************************************************************************
**** Interface
*******************************************************************************/
namespace cetech {

    /***************************************************************************
    **** Simple log system.
    ***************************************************************************/
    namespace log {

        /***********************************************************************
        **** Log level enum.
        ***********************************************************************/
        struct LogLevel {
            enum Enum {
                INFO,
                WARNING,
                ERROR,
                DBG
            };
        };

        /***********************************************************************
        **** Handler callback.
        ***********************************************************************/
        typedef void (* handler_t)(const LogLevel::Enum level,
                                   const time_t time,
                                   const uint32_t worker_id,
                                   const char* where,
                                   const char* msg,
                                   void* data);


        /***********************************************************************
        **** STDOUT handler. Print message to stdou and stderr
        ***********************************************************************/
        void stdout_handler(const LogLevel::Enum level,
                            const time_t time,
                            const uint32_t worker_id,
                            const char* where,
                            const char* msg,
                            void* data);

        /***********************************************************************
        **** File handler. Write message to file (data == *FILE).
        ***********************************************************************/
        void file_handler(const LogLevel::Enum level,
                          const time_t time,
                          const uint32_t worker_id,
                          const char* where,
                          const char* msg,
                          void* data);

        /***********************************************************************
        **** Register hadnler.
        ***********************************************************************/
        void register_handler(handler_t handler,
                              void* data = nullptr);

        /***********************************************************************
        **** Unregister handler.
        ***********************************************************************/
        void unregister_handler(handler_t handler);

        /***********************************************************************
        **** Info
        ***********************************************************************/
        void info(const char* where,
                  const char* format,
                  ...);

        void info(const char* where,
                  const char* format,
                  va_list va);

        /***********************************************************************
        **** Warning
        ***********************************************************************/
        void warning(const char* where,
                     const char* format,
                     ...);

        void warning(const char* where,
                     const char* format,
                     va_list va);

        /***********************************************************************
        **** Error
        ***********************************************************************/
        void error(const char* where,
                   const char* format,
                   ...);

        void error(const char* where,
                   const char* format,
                   va_list va);

        /***********************************************************************
        **** Debug
        ***********************************************************************/
        void debug(const char* where,
                   const char* format,
                   ...);

        void debug(const char* where,
                   const char* format,
                   va_list va);

    }

    /***************************************************************************
    **** Log system globals function.
    ***************************************************************************/
    namespace log_globals {
        /***********************************************************************
        **** Get worker clb.
        ***********************************************************************/
        typedef uint32_t (* get_worker_id_clb_t)();

        /***********************************************************************
        **** Init system.
        ***********************************************************************/
        void init(const get_worker_id_clb_t get_worker_id_clb);

        /***********************************************************************
        **** Shutdown system.
        ***********************************************************************/
        void shutdown();
    }
}
