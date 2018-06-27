#include <corelib/os.h>
#include <corelib/module.h>
#include <corelib/api_system.h>
#include "corelib/macros.h"


//#include "os_cpu.inl"
//#include "os_error.inl"
//#include "os_object.inl"
//#include "os_path.inl"
//#include "os_process.inl"
//#include "os_thread.inl"
//#include "os_time.inl"
//#include "os_vio.inl"
//#include "os_watchdog.inl"
//#include "os_window_sdl2.inl"

extern struct ct_cpu_a0 cpu_api;
extern struct ct_error_a0 error_api;
extern struct ct_object_a0 object_api;
extern struct ct_path_a0 path_api;
extern struct ct_process_a0 process_api;
extern struct ct_thread_a0 thread_api;
extern struct ct_time_a0 time_api;
extern struct ct_vio_a0 vio_api;
extern struct ct_watchdog_a0 wathdog_api;
extern struct ct_window_a0 window_api;


static struct ct_os_a0 ct_os_api = {
     .cpu_a0 = &cpu_api,
     .error_a0 = &error_api,
     .object_a0 = &object_api,
     .path_a0 = &path_api,
     .process_a0 = &process_api,
     .thread_a0 = &thread_api,
     .time_a0 = &time_api,
     .vio_a0 = &vio_api,
     .watchdog_a0 = &wathdog_api,
     .window_a0 = &window_api,
};

struct ct_os_a0* ct_os_a0 = &ct_os_api;

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
