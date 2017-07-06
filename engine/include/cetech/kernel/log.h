#ifndef CETECH_LOG_API_H
#define CETECH_LOG_API_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <time.h>
#include <stdarg.h>

#include "macros.h"

struct ct_api_a0;

//==============================================================================
// Enums
//==============================================================================

enum ct_log_level {
    LOG_INFO = 0,
    LOG_DBG = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3,
};

//==============================================================================
// Handlers
//==============================================================================

//! Stdout handler
void ct_log_stdout_handler(enum ct_log_level level,
                           time_t time,
                           char worker_id,
                           const char *where,
                           const char *msg,
                           void *data);


//! Nanomsg/mapack handler
void ct_nano_log_handler(enum ct_log_level level,
                         time_t time,
                         char worker_id,
                         const char *where,
                         const char *msg,
                         void *data);


//==============================================================================
// Typedefs
//==============================================================================

//! Log handler callback
typedef void (*ct_log_handler_t)(enum ct_log_level level,
                                 time_t time,
                                 char worker_id,
                                 const char *where,
                                 const char *msg,
                                 void *data);

//! Worker id callback
typedef char (*ct_log_get_wid_clb_t)();

//==============================================================================
// Api
//==============================================================================

struct ct_log_a0 {
    void (*set_wid_clb)(ct_log_get_wid_clb_t get_wid_clb);

    //! Register log handler
    //! \param handler Handler
    //! \param data Handler data
    void (*register_handler)(ct_log_handler_t handler,
                             void *data);

    //! Log info
    //! \param where Where
    //! \param format Format
    //! \param va va args
    void (*info_va)(const char *where,
                    const char *format,
                    va_list va);

    //! Log info
    //! \param where Where
    //! \param format Format
    void (*info)(const char *where,
                 const char *format,
                 ...) CTECH_ATTR_FORMAT(2, 3);

    //! Log warnig
    //! \param where Where
    //! \param format Format
    //! \param va va args
    void (*warning_va)(const char *where,
                       const char *format,
                       va_list va);

    //! Log warning
    //! \param where Where
    //! \param format Format
    void (*warning)(const char *where,
                    const char *format,
                    ...) CTECH_ATTR_FORMAT(2, 3);

    //! Log error
    //! \param where Where
    //! \param format Format
    //! \param va va args
    void (*error_va)(const char *where,
                     const char *format,
                     va_list va);

    //! Log error
    //! \param where Where
    //! \param format Format
    void (*error)(const char *where,
                  const char *format,
                  ...) CTECH_ATTR_FORMAT(2, 3);

    //! Log debug
    //! \param where Where
    //! \param format Format
    //! \param va va args
    void (*debug_va)(const char *where,
                     const char *format,
                     va_list va);

    //! Log debug
    //! \param where Where
    //! \param format Format
    void (*debug)(const char *where,
                  const char *format,
                  ...) CTECH_ATTR_FORMAT(2, 3);

};

#ifdef __cplusplus
}
#endif

#endif //CETECH_LOG_API_H
