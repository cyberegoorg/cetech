#include <stdbool.h>
#include <cetech/core/module.h>
#include "allocator_core_private.h"
#include "api_private.h"
#include "memory_private.h"
#include "log_system_private.h"

bool celib_init() {
    struct ct_alloc *core_alloc = coreallocator_get();

    api_init(core_alloc);
    struct ct_api_a0 *api = api_v0();

    CETECH_LOAD_STATIC_MODULE(api, log);

    memory_init();

    coreallocator_register_api(api);

    CETECH_LOAD_STATIC_MODULE(api, hashlib);

    register_api(api);

    CETECH_LOAD_STATIC_MODULE(api, error);
    CETECH_LOAD_STATIC_MODULE(api, vio);
    CETECH_LOAD_STATIC_MODULE(api, process);
    CETECH_LOAD_STATIC_MODULE(api, cpu);
    CETECH_LOAD_STATIC_MODULE(api, time);
    CETECH_LOAD_STATIC_MODULE(api, thread);
    CETECH_LOAD_STATIC_MODULE(api, path);
    CETECH_LOAD_STATIC_MODULE(api, task);
    CETECH_LOAD_STATIC_MODULE(api, watchdog);
    CETECH_LOAD_STATIC_MODULE(api, filesystem);

    CETECH_LOAD_STATIC_MODULE(api, object);

    return true;
}

bool celib_shutdown() {
    struct ct_api_a0 *api = api_v0();

    CETECH_UNLOAD_STATIC_MODULE(api, error);
    CETECH_UNLOAD_STATIC_MODULE(api, vio);
    CETECH_UNLOAD_STATIC_MODULE(api, process);
    CETECH_UNLOAD_STATIC_MODULE(api, cpu);
    CETECH_UNLOAD_STATIC_MODULE(api, time);
    CETECH_UNLOAD_STATIC_MODULE(api, thread);
    CETECH_UNLOAD_STATIC_MODULE(api, path);
    CETECH_UNLOAD_STATIC_MODULE(api, object);
    CETECH_UNLOAD_STATIC_MODULE(api, module);
    CETECH_UNLOAD_STATIC_MODULE(api, task);
    CETECH_UNLOAD_STATIC_MODULE(api, watchdog);

    api_shutdown();
    memsys_shutdown();
    logsystem_shutdown();

    return true;
}
