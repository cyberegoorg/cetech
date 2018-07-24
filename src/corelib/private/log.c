#include <stdio.h>
#include <stdint.h>
#include <corelib/log.h>
#include <corelib/module.h>
#include <corelib/api_system.h>
#include <corelib/task.h>

#include "corelib/macros.h"


#define LOG_WHERE "log_system"

enum {
    MAX_HANDLERS = 32
};

static struct global {
    ct_log_handler_t handlers[MAX_HANDLERS];
    void *handlers_data[MAX_HANDLERS];

    char handlers_count;
} _G = {{0}};


static void vlog(const enum ct_log_level level,
                 const char *where,
                 const char *format,
                 va_list va) {

    //CE_ASSERT("logsystem", _globals.data != nullptr);

    char msg[4096];     //!< Final msg.
    vsnprintf(msg, 4096, format, va);

    time_t tm = time(NULL);

    for (uint8_t i = 0; i < _G.handlers_count; ++i) {
        _G.handlers[i](level, tm, ct_task_a0->worker_id(),
                       where, msg, _G.handlers_data[i]);
    }
}

static void log_info_va(const char *where,
                        const char *format,
                        va_list va) {
    vlog(LOG_INFO, where, format, va);
}


static void log_info(const char *where,
                     const char *format,
                     ...) {
    va_list args;

    va_start(args, format);
    log_info_va(where, format, args);
    va_end(args);
}

static void log_warning_va(const char *where,
                           const char *format,
                           va_list va) {
    vlog(LOG_WARNING, where, format, va);
}

static void log_warning(const char *where,
                        const char *format,
                        ...) {
    va_list args;

    va_start(args, format);
    log_warning_va(where, format, args);
    va_end(args);
}

static void log_error_va(const char *where,
                         const char *format,
                         va_list va) {
    vlog(LOG_ERROR, where, format, va);
}

static void log_error(const char *where,
                      const char *format,
                      ...) {
    va_list args;

    va_start(args, format);
    log_error_va(where, format, args);
    va_end(args);
}

static void log_debug_va(const char *where,
                         const char *format,
                         va_list va) {

    vlog(LOG_DBG, where, format, va);
}

static void log_debug(const char *where,
                      const char *format,
                      ...) {
    va_list args;

    va_start(args, format);
    log_debug_va(where, format, args);
    va_end(args);
}


static void log_register_handler(ct_log_handler_t handler,
                                 void *data) {
    const uint8_t idx = _G.handlers_count++;

    _G.handlers[idx] = handler;
    _G.handlers_data[idx] = data;
}



void ct_log_stdout_yaml_handler(enum ct_log_level level,
                                time_t time,
                                char worker_id,
                                const char *where,
                                const char *msg,
                                void *data);

void ct_log_stdout_handler(enum ct_log_level level,
                                time_t time,
                                char worker_id,
                                const char *where,
                                const char *msg,
                                void *data);


static struct ct_log_a0 log_a0 = {
        .stdout_handler = &ct_log_stdout_handler,
        .stdout_yaml_handler = &ct_log_stdout_yaml_handler,
        .register_handler = log_register_handler,
        .info_va = log_info_va,
        .info = log_info,
        .warning_va = log_warning_va,
        .warning = log_warning,
        .error_va = log_error_va,
        .error = log_error,
        .debug_va = log_debug_va,
        .debug = log_debug
};

struct ct_log_a0 *ct_log_a0 = &log_a0;

CETECH_MODULE_DEF(
        log,
        {
            CT_UNUSED(api);
        },
        {
            CT_UNUSED(reload);

        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
        }
)

