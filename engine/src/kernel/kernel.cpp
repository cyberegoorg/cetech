#include <cetech/celib/allocator.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/task.h>

#include "log_system_private.h"
#include "memory_private.h"
#include "api_private.h"
#include "module_private.h"
#include "config_private.h"

#include "../modules/static_module.h"
#include "allocator_core_private.h"

CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_task_a0);

#define LOG_WHERE "kernel"


namespace os {
    void register_api(ct_api_a0 *api);
}

void application_start();

const char *_platform() {
#if defined(CETECH_LINUX)
    return "linux";
#elif defined(CETECH_WINDOWS)
    return "windows";
#elif defined(CETECH_DARWIN)
    return "darwin";
#endif
    return NULL;
}

int load_config(int argc,
                const char **argv) {

#ifdef CETECH_CAN_COMPILE
    if (!config::parse_args(argc, argv)) {
        return 0;
    }

    ct_cvar compile = config::find("compile");
    if (config::get_int(compile)) {
        config::compile_global(_platform());
    }
#endif

    config::load_global(_platform());

    if (!config::parse_args(argc, argv)) {
        return 0;
    }

    config::log_all();

    return 1;
}

void application_register_api(ct_api_a0 *api);

extern "C" void init_core(ct_api_a0 *api) {
    auto *core_alloc = core_allocator::get();

    LOAD_STATIC_MODULE(api, hashlib);

    log::register_api(api);
    core_allocator::register_api(api);
    memory::register_api(api);
    application_register_api(api);

    os::register_api(api);
    LOAD_STATIC_MODULE(api, os);

    module::init(core_alloc, api);
}

extern "C" void load_core() {
    ADD_STATIC_MODULE(developsystem);

    ADD_STATIC_MODULE(task);
    ADD_STATIC_MODULE(consoleserver);
}

int cetech_kernel_init(int argc,
                       const char **argv) {
    auto *core_alloc = core_allocator::get();

    log::init();
    api::init(core_alloc);
    ct_api_a0 *api = api::v0();

    memory::init(4 * 1024 * 1024);

    init_core(api);
    config::init(api);

    log::logdb_init_db(".", api);
    load_config(argc, argv);

    load_core();
    init_static_modules();

    module::load_dirs("./bin");

    CETECH_GET_API(api, ct_log_a0);
    return 1;
}


int cetech_kernel_shutdown() {
    ct_log_a0.debug(LOG_WHERE, "Shutdown");

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
