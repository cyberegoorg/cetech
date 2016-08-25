#ifndef CETECH_LOG_API_H
#define CETECH_LOG_API_H

//==============================================================================
// Includes
//==============================================================================

#include <time.h>
#include <stdio.h>

#include "celib/types.h"

//==============================================================================
// Enums
//==============================================================================

enum log_level {
    LOG_INFO = 0,
    LOG_DBG = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3,
};

//==============================================================================
// Handlers
//==============================================================================

//! Stdout handler
void log_stdout_handler(enum log_level level,
                        time_t time,
                        char worker_id,
                        const char *where,
                        const char *msg,
                        void *data);


//! Nanomsg/mapack handler
void nano_log_handler(enum log_level level,
                      time_t time,
                      char worker_id,
                      const char *where,
                      const char *msg,
                      void *data);

//==============================================================================
// Typedefs
//==============================================================================

//! Log handler callback
typedef void (*log_handler_t)(enum log_level level,
                              time_t time,
                              char worker_id,
                              const char *where,
                              const char *msg,
                              void *data);

//! Worker id callback
typedef char (*log_get_wid_clb_t)();

//==============================================================================
// Interface
//==============================================================================

//! Init log system
//! \param get_wid_clb Get worker id callback
void log_init(log_get_wid_clb_t get_wid_clb);

//! Shutdown log system
void log_shutdown();

void log_set_wid_clb(log_get_wid_clb_t get_wid_clb);

//! Register log handler
//! \param handler Handler
//! \param data Handler data
void log_register_handler(log_handler_t handler, void *data);

//! Log info
//! \param where Where
//! \param format Format
//! \param va va args
void log_info_va(const char *where,
                 const char *format,
                 va_list va);

//! Log info
//! \param where Where
//! \param format Format
void log_info(const char *where,
              const char *format,
              ...) ATTR_FORMAT(2, 3);

//! Log warnig
//! \param where Where
//! \param format Format
//! \param va va args
void log_warning_va(const char *where,
                    const char *format,
                    va_list va);

//! Log warning
//! \param where Where
//! \param format Format
void log_warning(const char *where,
                 const char *format,
                 ...) ATTR_FORMAT(2, 3);

//! Log error
//! \param where Where
//! \param format Format
//! \param va va args
void log_error_va(const char *where,
                  const char *format,
                  va_list va);

//! Log error
//! \param where Where
//! \param format Format
void log_error(const char *where,
               const char *format,
               ...) ATTR_FORMAT(2, 3);

//! Log debug
//! \param where Where
//! \param format Format
//! \param va va args
void log_debug_va(const char *where,
                  const char *format,
                  va_list va);

//! Log debug
//! \param where Where
//! \param format Format
void log_debug(const char *where,
               const char *format,
               ...) ATTR_FORMAT(2, 3);

#endif //CETECH_LOG_API_H
