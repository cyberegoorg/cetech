#include <cetech/celib/allocator.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/application.h>
#include <cetech/kernel/task.h>

#include "../log/log_system_private.h"
#include "../memory/memory_private.h"
#include "../api/api_private.h"
#include "../os_sdl2/sdl2_os_private.h"
#include "../module/module_private.h"
#include "../config/config_private.h"
#include "static_module.h"

CETECH_DECL_API(app_api_v0);
CETECH_DECL_API(log_api_v0);
CETECH_DECL_API(task_api_v0);


#define LOG_WHERE "application"

int application_init(int argc,
                     const char **argv);

void application_start();

int application_shutdown();

extern void error_register_api(struct api_v0 *api);

extern "C" void _init_core_modules() {
    ADD_STATIC_MODULE(blob);
    ADD_STATIC_MODULE(os);
    ADD_STATIC_MODULE(task);
    ADD_STATIC_MODULE(developsystem);
    ADD_STATIC_MODULE(consoleserver);
    ADD_STATIC_MODULE(filesystem);
    ADD_STATIC_MODULE(resourcesystem);
#ifdef CETECH_CAN_COMPILE
    ADD_STATIC_MODULE(resourcecompiler);
#endif
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

extern "C" void init_hashlib(struct api_v0 *api) {
    LOAD_STATIC_MODULE(api, hashlib);
}

int cetech_kernel_init(int argc,
                       const char **argv) {

    log::init();
    memory::memsys_init(4 * 1024 * 1024);

    allocator *a = memory::memsys_main_allocator();

    api::init(a);
    api_v0 *api = api::v0();

    log::register_api(api);
    memory::register_api(api);
    error_register_api(api);

    init_hashlib(api);
    application_register_api(api);

    os::register_api(api);
    module::init(a, api);

    CETECH_GET_API(api, app_api_v0);
    config::init(api);

    log::logdb_init_db(".", api);
    init_config(argc, argv);

    _init_core_modules();
    CETECH_GET_API(api, log_api_v0);
    CETECH_GET_API(api, task_api_v0);
    log_api_v0.set_wid_clb(task_api_v0.worker_id);

    _init_static_modules();

    module::load_dirs("./bin");

    return 1;
}


int cetech_kernel_shutdown() {
    api_v0 *api = api::v0();

    log_api_v0.debug(LOG_WHERE, "Shutdown");

    module::unload_all();

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
