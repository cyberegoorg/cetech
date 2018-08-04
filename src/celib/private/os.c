#include <celib/os.h>
#include <celib/module.h>
#include <celib/api_system.h>
#include "celib/macros.h"

extern struct ce_os_cpu_a0 cpu_api;
extern struct ce_os_error_a0 error_api;
extern struct ce_os_object_a0 object_api;
extern struct ce_os_path_a0 path_api;
extern struct ce_os_process_a0 process_api;
extern struct ce_os_thread_a0 thread_api;
extern struct ce_os_time_a0 time_api;
extern struct ce_os_vio_a0 vio_api;
extern struct ce_os_window_a0 window_api;


static struct ce_os_a0 ct_os_api = {
        .cpu = &cpu_api,
        .error = &error_api,
        .object = &object_api,
        .path = &path_api,
        .process = &process_api,
        .thread = &thread_api,
        .time = &time_api,
        .vio = &vio_api,
        .window = &window_api,
};

struct ce_os_a0 *ce_os_a0 = &ct_os_api;

CE_MODULE_DEF(
        os,
        {
            CE_UNUSED(api);
        },
        {
            CE_UNUSED(reload);
            api->register_api("ce_os_a0", ce_os_a0);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
        }
)
