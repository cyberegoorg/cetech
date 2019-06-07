#include <time.h>

#include <celib/log.h>
#include <celib/module.h>
#include <celib/api.h>
#include <celib/task.h>

#include "celib/macros.h"

#include "celib/id.h"

#define LOG_WHERE "log_system"

enum {
    MAX_HANDLERS = 32
};

static struct global {
    ce_log_handler_t handlers[MAX_HANDLERS];
    void *handlers_data[MAX_HANDLERS];

    char handlers_count;
} _G = {};


static void vlog(const enum ce_log_level_e0 level,
                 const char *where,
                 const char *format,
                 va_list va) {

    //CE_ASSERT("logsystem", _globals.data != nullptr);

    va_list cva;
    va_copy(cva, va);
    int len = vsnprintf(0, 0, format, cva);
    va_end(cva);

    char msg[len + 1];
    int s = vsnprintf(msg, CE_ARRAY_LEN(msg), format, va);

    //TODO:
    if (level == LOG_ERROR) {
        char *st = ce_os_error_a0->stacktrace(4);
        snprintf(msg + s, CE_ARRAY_LEN(msg) - s, "\n    stacktrace:\n%s\n", st);
        ce_os_error_a0->stacktrace_free(st);
    }

    time_t tm = time(NULL);

    for (uint8_t i = 0; i < _G.handlers_count; ++i) {
        _G.handlers[i](level, *(ce_time_t *) &tm, ce_task_a0->worker_id(),
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


static void log_register_handler(ce_log_handler_t handler,
                                 void *data) {
    const uint8_t idx = _G.handlers_count++;

    _G.handlers[idx] = handler;
    _G.handlers_data[idx] = data;
}


void ct_log_stdout_yaml_handler(enum ce_log_level_e0 level,
                                ce_time_t time,
                                char worker_id,
                                const char *where,
                                const char *msg,
                                void *data);

void ct_log_stdout_handler(enum ce_log_level_e0 level,
                           ce_time_t time,
                           char worker_id,
                           const char *where,
                           const char *msg,
                           void *data);


static struct ce_log_a0 log_a0 = {
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

struct ce_log_a0 *ce_log_a0 = &log_a0;

void CE_MODULE_LOAD(log)(struct ce_api_a0 *api,
                         int reload) {
    CE_UNUSED(reload);
    api->register_api(CE_LOG_API, ce_log_a0, sizeof(log_a0));
}

void CE_MODULE_UNLOAD(log)(struct ce_api_a0 *api,
                           int reload) {
    CE_UNUSED(reload);
    CE_UNUSED(api);
}


