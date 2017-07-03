#include <cetech/celib/allocator.h>
#include <cetech/kernel/api.h>
#include <cetech/kernel/application.h>
#include <cetech/kernel/task.h>

#include "log/log_system_private.h"
#include "memory/memory_private.h"
#include "api/_api.h"
#include "os/os_private.h"
#include "module/module_private.h"
#include "config/config_private.h"
#include "static_module.h"

CETECH_DECL_API(app_api_v0);
CETECH_DECL_API(log_api_v0);
CETECH_DECL_API(task_api_v0);


#define LOG_WHERE "application"

int application_init(int argc,
                     const char **argv);

void application_start();

int application_shutdown();

extern void error_init(struct api_v0 *api);

extern "C" void _init_core_modules(struct api_v0 *api) {
    LOAD_STATIC_MODULE(api, os);
    LOAD_STATIC_MODULE(api, blob);
    LOAD_STATIC_MODULE(api, machine);
    LOAD_STATIC_MODULE(api, task);
    LOAD_STATIC_MODULE(api, developsystem);
    LOAD_STATIC_MODULE(api, consoleserver);

    LOAD_STATIC_MODULE(api, filesystem);
    LOAD_STATIC_MODULE(api, resourcesystem);

#ifdef CETECH_CAN_COMPILE
    LOAD_STATIC_MODULE(api, resourcecompiler);
#endif
}

extern "C" void _shutdown_core_modules(struct api_v0 *api) {
#ifdef CETECH_CAN_COMPILE
    LOAD_STATIC_MODULE(api, resourcecompiler);
#endif
    UNLOAD_STATIC_MODULE(api, resourcesystem);
    UNLOAD_STATIC_MODULE(api, filesystem);
    UNLOAD_STATIC_MODULE(api, consoleserver);
    UNLOAD_STATIC_MODULE(api, developsystem);
    UNLOAD_STATIC_MODULE(api, task);
    UNLOAD_STATIC_MODULE(api, machine);
    UNLOAD_STATIC_MODULE(api, blob);

    UNLOAD_STATIC_MODULE(api, os);
}


int init_config(int argc,
                const char **argv) {

#ifdef CETECH_CAN_COMPILE
    if (!config::parse_args(argc, argv)) {
        return 0;
    }

    cvar_t compile = config::find("compile");
    if (config::get_int(compile)) {
        config::compile_global(&app_api_v0);
    }
#endif

    config::load_global(&app_api_v0);

    if (!config::parse_args(argc, argv)) {
        return 0;
    }

    config::log_all();

    return 1;
}

void application_register_api(struct api_v0 *api);

int cetech_kernel_init(int argc,
                       const char **argv) {

    log::init();
    memory::memsys_init(4 * 1024 * 1024);

    allocator *a = memory::_memsys_main_allocator();

    api::init(a);

    api_v0 *api = api::v0();

    log::register_api(api);
    error_init(api);


    memory::memsys_init_api(api);
    os::init(api);
    application_register_api(api);
    CETECH_GET_API(api, app_api_v0);

    module::init(a, api);
    config::init(api);

    log::logdb_init_db(".", api);

    init_config(argc, argv);

    _init_core_modules(api);

    CETECH_GET_API(api, log_api_v0);
    CETECH_GET_API(api, task_api_v0);
    log_api_v0.set_wid_clb(task_api_v0.worker_id);

    _init_static_modules(api);

    //module::load_dirs("./bin");
    //module::call_init();


    return 1;
}


int cetech_kernel_shutdown() {
    api_v0 *api = api::v0();

    log_api_v0.debug(LOG_WHERE, "Shutdown");

    _shutdown_static_modules(api);
    _shutdown_core_modules(api);
    //module::call_shutdown();

    config::shutdown();
    module::shutdown();
    api::shutdown();
    memory::memsys_shutdown();
    log::shutdown();

    return 1;
}

int main(int argc,
         const char **argv) {

    if (cetech_kernel_init(argc, argv)) {
        application_start();
    }

    return cetech_kernel_shutdown();
}
