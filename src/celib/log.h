#ifndef CE_LOG_API_H
#define CE_LOG_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib_types.h"

#include <celib/macros.h>

#define CE_LOG_API \
    CE_ID64_0("ce_log_a0", 0x1e2a79ce44a2031cULL)

typedef enum ce_log_level_e0 {
    LOG_INFO = 0,
    LOG_DBG = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3,
} ce_log_level_e0;

//! Log handler callback
typedef void (*ce_log_handler_t)(enum ce_log_level_e0 level,
                                 ce_time_t time,
                                 char worker_id,
                                 const char *where,
                                 const char *msg,
                                 void *data);

struct ce_log_a0 {
    ce_log_handler_t stdout_yaml_handler;
    ce_log_handler_t stdout_handler;

    //! Register log handler
    //! \param handler Handler
    //! \param data Handler data
    void (*register_handler)(ce_log_handler_t handler,
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

CE_MODULE(ce_log_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_LOG_API_H
