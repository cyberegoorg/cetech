#include <stdio.h>
#include <stdint.h>

#include <cetech/kernel/log.h>
#include <cetech/kernel/errors.h>
#include <cetech/kernel/api_system.h>

#define LOG_WHERE "log_system"

enum {
    MAX_HANDLERS = 32
};

#define G_INIT {0}

static struct global {
    ct_log_handler_t handlers[MAX_HANDLERS];
    void *handlers_data[MAX_HANDLERS];

    ct_log_get_wid_clb_t get_wid_clb;
    char handlers_count;
} _G = G_INIT;


void vlog(const enum ct_log_level level,
          const char *where,
          const char *format,
          va_list va) {

    //CE_ASSERT("log", _globals.data != nullptr);

    char msg[4096];     //!< Final msg.
    vsnprintf(msg, 4096, format, va);

    time_t tm = time(NULL);

    for (uint32_t i = 0; i < _G.handlers_count; ++i) {
        _G.handlers[i](level, tm, _G.get_wid_clb != NULL ? _G.get_wid_clb() : 0,
                       where, msg, _G.handlers_data[i]);
    }
}

namespace log {

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


    void log_register_handler(ct_log_handler_t handler,
                              void *data) {
        const char idx = _G.handlers_count++;

        _G.handlers[idx] = handler;
        _G.handlers_data[idx] = data;
    }

    void init() {
        _G = (struct global) G_INIT;
        log_register_handler(ct_log_stdout_handler, NULL);
    }

    void shutdown() {
        _G = (struct global) G_INIT;
    }


    void set_wid_clb(ct_log_get_wid_clb_t get_wid_clb) {
        _G.get_wid_clb = get_wid_clb;
    }

    static ct_log_a0 log_a0 = {
            .set_wid_clb = log::set_wid_clb,
            .register_handler = log::log_register_handler,
            .info_va = log_info_va,
            .info = log_info,
            .warning_va = log_warning_va,
            .warning = log_warning,
            .error_va = log_error_va,
            .error = log_error,
            .debug_va = log_debug_va,
            .debug = log_debug
    };


    void register_api(ct_api_a0 *api) {
        api->register_api("ct_log_a0", &log_a0);
    }

}
