#include <stdbool.h>
#include <corelib/module.h>
#include <corelib/memory.h>
#include <corelib/api_system.h>
#include "corelib/private/api_private.h"
#include "corelib/private/memory_private.h"
#include "corelib/private/log_system_private.h"

void coreallocator_register_api(struct ct_api_a0 *api);

bool ct_corelib_init() {
    struct ct_alloc *core_alloc = ct_core_allocator_a0->alloc;

    memory_init();

    api_init(core_alloc);

    CETECH_LOAD_STATIC_MODULE(ct_api_a0, log);

    coreallocator_register_api(ct_api_a0);
    register_api(ct_api_a0);

    CETECH_LOAD_STATIC_MODULE(ct_api_a0, hashlib);

    CETECH_LOAD_STATIC_MODULE(ct_api_a0, os);
    CETECH_LOAD_STATIC_MODULE(ct_api_a0, task);
    CETECH_LOAD_STATIC_MODULE(ct_api_a0, cdb);
    CETECH_LOAD_STATIC_MODULE(ct_api_a0, ebus);
    CETECH_LOAD_STATIC_MODULE(ct_api_a0, yamlng);
    CETECH_LOAD_STATIC_MODULE(ct_api_a0, config);
    CETECH_LOAD_STATIC_MODULE(ct_api_a0, filesystem);
    CETECH_LOAD_STATIC_MODULE(ct_api_a0, module);
    CETECH_LOAD_STATIC_MODULE(ct_api_a0, ydb);

    return true;
}

bool ct_corelib_shutdown() {
    CETECH_UNLOAD_STATIC_MODULE(ct_api_a0, os);
    CETECH_UNLOAD_STATIC_MODULE(ct_api_a0, task);
    CETECH_UNLOAD_STATIC_MODULE(ct_api_a0, yamlng);
    CETECH_UNLOAD_STATIC_MODULE(ct_api_a0, config);

    CETECH_UNLOAD_STATIC_MODULE(ct_api_a0, filesystem);
    CETECH_UNLOAD_STATIC_MODULE(ct_api_a0, cdb);
    CETECH_UNLOAD_STATIC_MODULE(ct_api_a0, module);
    // CETECH_UNLOAD_STATIC_MODULE(ct_api_a0, ydb);

    api_shutdown();
    memsys_shutdown();
    logsystem_shutdown();

    return true;
}
