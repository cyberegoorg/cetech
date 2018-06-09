#include <stdbool.h>
#include <cetech/kernel/module/module.h>
#include "cetech/kernel/memory/private/allocator_core_private.h"
#include "cetech/kernel/api/private/api_private.h"
#include "cetech/kernel/memory/private/memory_private.h"
#include "cetech/kernel/log/private/log_system_private.h"

bool corelib_init() {
    struct ct_alloc *core_alloc = coreallocator_get();

    memory_init();

    api_init(core_alloc);
    struct ct_api_a0 *api = api_v0();

    CETECH_LOAD_STATIC_MODULE(api, log);

    coreallocator_register_api(api);
    register_api(api);

    CETECH_LOAD_STATIC_MODULE(api, hashlib);

    CETECH_LOAD_STATIC_MODULE(api, error);
    CETECH_LOAD_STATIC_MODULE(api, vio);
    CETECH_LOAD_STATIC_MODULE(api, process);
    CETECH_LOAD_STATIC_MODULE(api, cpu);
    CETECH_LOAD_STATIC_MODULE(api, time);
    CETECH_LOAD_STATIC_MODULE(api, thread);
    CETECH_LOAD_STATIC_MODULE(api, path);
    CETECH_LOAD_STATIC_MODULE(api, task);
    CETECH_LOAD_STATIC_MODULE(api, cdb);
    CETECH_LOAD_STATIC_MODULE(api, ebus);
    CETECH_LOAD_STATIC_MODULE(api, watchdog);
    CETECH_LOAD_STATIC_MODULE(api, yamlng);
    CETECH_LOAD_STATIC_MODULE(api, config);
    CETECH_LOAD_STATIC_MODULE(api, object);
    CETECH_LOAD_STATIC_MODULE(api, filesystem);
    CETECH_LOAD_STATIC_MODULE(api, module);
    CETECH_LOAD_STATIC_MODULE(api, ydb);

    return true;
}

bool corelib_shutdown() {
    struct ct_api_a0 *api = api_v0();

    CETECH_UNLOAD_STATIC_MODULE(api, error);
    CETECH_UNLOAD_STATIC_MODULE(api, vio);
    CETECH_UNLOAD_STATIC_MODULE(api, process);
    CETECH_UNLOAD_STATIC_MODULE(api, cpu);
    CETECH_UNLOAD_STATIC_MODULE(api, time);
    CETECH_UNLOAD_STATIC_MODULE(api, thread);
    CETECH_UNLOAD_STATIC_MODULE(api, path);
    CETECH_UNLOAD_STATIC_MODULE(api, task);
    CETECH_UNLOAD_STATIC_MODULE(api, watchdog);
    CETECH_UNLOAD_STATIC_MODULE(api, yamlng);
    CETECH_UNLOAD_STATIC_MODULE(api, config);
    CETECH_UNLOAD_STATIC_MODULE(api, object);
    CETECH_UNLOAD_STATIC_MODULE(api, filesystem);
    CETECH_UNLOAD_STATIC_MODULE(api, cdb);
    CETECH_UNLOAD_STATIC_MODULE(api, module);
   // CETECH_UNLOAD_STATIC_MODULE(api, ydb);

    api_shutdown();
    memsys_shutdown();
    logsystem_shutdown();

    return true;
}
