

#include <celib/module.h>
#include <celib/api.h>
#include "celib/macros.h"

#include <celib/os/cpu.h>
#include <celib/os/error.h>
#include <celib/os/input.h>
#include <celib/os/object.h>
#include <celib/os/path.h>
#include <celib/os/process.h>
#include <celib/os/thread.h>
#include <celib/os/time.h>
#include <celib/os/vio.h>
#include <celib/os/window.h>

extern struct ce_os_cpu_a0 cpu_api;
extern struct ce_os_error_a0 error_api;
extern struct ce_os_object_a0 object_api;
extern struct ce_os_path_a0 path_api;
extern struct ce_os_process_a0 process_api;
extern struct ce_os_thread_a0 thread_api;
extern struct ce_os_time_a0 time_api;
extern struct ce_os_vio_a0 vio_api;
extern struct ce_os_window_a0 window_api;
extern struct ce_os_input_a0 input_api;


void CE_MODULE_LOAD(os)(struct ce_api_a0 *api,
                        int reload) {
    CE_UNUSED(reload);

    api->add_api(CE_OS_PATH_API, &path_api, sizeof(path_api));
    api->add_api(CE_OS_CPU_API, &cpu_api, sizeof(cpu_api));
    api->add_api(CE_OS_ERROR_API, &error_api, sizeof(error_api));
    api->add_api(CE_OS_OBJECT_API, &object_api, sizeof(object_api));
    api->add_api(CE_OS_PROCESS_API, &process_api, sizeof(process_api));
    api->add_api(CE_OS_TIME_API, &time_api, sizeof(time_api));
    api->add_api(CE_OS_VIO_API, &vio_api, sizeof(vio_api));
    api->add_api(CE_OS_WINDOW_API, &window_api, sizeof(window_api));
    api->add_api(CE_OS_INPUT_API, &input_api, sizeof(input_api));
    api->add_api(CE_OS_THREAD_API, &thread_api, sizeof(thread_api));

}

void CE_MODULE_UNLOAD(os)(struct ce_api_a0 *api,
                          int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
