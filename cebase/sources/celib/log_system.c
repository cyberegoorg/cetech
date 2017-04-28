#include <stdio.h>
#include <stdint.h>
#include <celib/log.h>

#include "celib/errors.h"

#define LOG_WHERE "log_system"

///
enum {
    MAX_HANDLERS = 32
};

#define G_INIT {0}

static struct global {
    log_handler_t handlers[MAX_HANDLERS];
    void *handlers_data[MAX_HANDLERS];

    log_get_wid_clb_t get_wid_clb;
    char handlers_count;
} _G = G_INIT;

void vlog(const enum log_level level,
          const char *where,
          const char *format,
          va_list va) {

    //CE_ASSERT("log", _globals.data != nullptr);

    char msg[4096];     //!< Final msg.
    vsnprintf(msg, 4096, format, va);

    time_t tm = time(NULL);

    for (uint32_t i = 0; i < _G.handlers_count; ++i) {
        CEL_ASSERT("log", _G.handlers[i] != NULL);

        _G.handlers[i](level, tm, _G.get_wid_clb != NULL ? _G.get_wid_clb() : 0, where, msg, _G.handlers_data[i]);
    }
}

void log_init(log_get_wid_clb_t get_wid_clb) {
    _G = (struct global) G_INIT;
}

void log_shutdown() {
    _G = (struct global) G_INIT;
}

void log_set_wid_clb(log_get_wid_clb_t get_wid_clb) {
    _G.get_wid_clb = get_wid_clb;
}

void log_register_handler(log_handler_t handler,
                          void *data) {
    const char idx = _G.handlers_count++;

    _G.handlers[idx] = handler;
    _G.handlers_data[idx] = data;
}

void log_info_va(const char *where,
                 const char *format,
                 va_list va) {
    vlog(LOG_INFO, where, format, va);
}


void log_info(const char *where,
              const char *format,
              ...) {
    va_list args;

    va_start(args, format);
    log_info_va(where, format, args);
    va_end(args);
}

void log_warning_va(const char *where,
                    const char *format,
                    va_list va) {
    vlog(LOG_WARNING, where, format, va);
}

void log_warning(const char *where,
                 const char *format,
                 ...) {
    va_list args;

    va_start(args, format);
    log_warning_va(where, format, args);
    va_end(args);
}

void log_error_va(const char *where,
                  const char *format,
                  va_list va) {
    vlog(LOG_ERROR, where, format, va);
}

void log_error(const char *where,
               const char *format,
               ...) {
    va_list args;

    va_start(args, format);
    log_error_va(where, format, args);
    va_end(args);
}

void log_debug_va(const char *where,
                  const char *format,
                  va_list va) {

    vlog(LOG_DBG, where, format, va);
}

void log_debug(const char *where,
               const char *format,
               ...) {
    va_list args;

    va_start(args, format);
    log_debug_va(where, format, args);
    va_end(args);
}