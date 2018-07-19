#include <corelib/os.h>
#include <corelib/module.h>
#include <corelib/api_system.h>
#include "corelib/macros.h"

extern struct ct_os_cpu_a0 cpu_api;
extern struct ct_os_error_a0 error_api;
extern struct ct_os_object_a0 object_api;
extern struct ct_os_path_a0 path_api;
extern struct ct_os_process_a0 process_api;
extern struct ct_os_thread_a0 thread_api;
extern struct ct_os_time_a0 time_api;
extern struct ct_os_vio_a0 vio_api;
extern struct ct_os_watchdog_a0 wathdog_api;
extern struct ct_os_window_a0 window_api;


static struct ct_os_a0 ct_os_api = {
        .cpu = &cpu_api,
        .error = &error_api,
        .object = &object_api,
        .path = &path_api,
        .process = &process_api,
        .thread = &thread_api,
        .time = &time_api,
        .vio = &vio_api,
        .watchdog = &wathdog_api,
        .window = &window_api,
};

struct ct_os_a0 *ct_os_a0 = &ct_os_api;

CETECH_MODULE_DEF(
        os,
        {
            CT_UNUSED(api);
        },
        {
            CT_UNUSED(reload);
            api->register_api("ct_os_a0", ct_os_a0);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
        }
)
