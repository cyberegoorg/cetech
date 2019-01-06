#include <stdbool.h>
#include <celib/module.h>
#include <celib/memory.h>
#include <celib/api_system.h>
#include <celib/log.h>
#include <signal.h>
#include <celib/os.h>
#include <stdlib.h>
#include "celib/private/api_private.h"

void memory_register_api(struct ce_api_a0 *api);

struct sigaction sigact;

static void signal_handler(int sig) {
    if (sig == SIGSEGV){
        ce_log_a0->error("hell", "");
        exit(-1);
    }
}

void init_signals(void) {
    sigact.sa_handler = signal_handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;

    sigaddset(&sigact.sa_mask, SIGSEGV);
    sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL);
}

bool ce_init() {
    struct ce_alloc *core_alloc =ce_memory_a0->system;

    api_init(core_alloc);

    CE_LOAD_STATIC_MODULE(ce_api_a0, log);
    
    memory_register_api(ce_api_a0);

    CE_LOAD_STATIC_MODULE(ce_api_a0, hashlib);
    CE_LOAD_STATIC_MODULE(ce_api_a0, os);

    init_signals();

    CE_LOAD_STATIC_MODULE(ce_api_a0, task);
    CE_LOAD_STATIC_MODULE(ce_api_a0, cdb);
    CE_LOAD_STATIC_MODULE(ce_api_a0, config);
    CE_LOAD_STATIC_MODULE(ce_api_a0, filesystem);
    CE_LOAD_STATIC_MODULE(ce_api_a0, module);
    CE_LOAD_STATIC_MODULE(ce_api_a0, ydb);

    return true;
}

void memsys_shutdown();

bool ce_shutdown() {

    CE_UNLOAD_STATIC_MODULE(ce_api_a0, os);
    CE_UNLOAD_STATIC_MODULE(ce_api_a0, task);
    CE_UNLOAD_STATIC_MODULE(ce_api_a0, config);

    CE_UNLOAD_STATIC_MODULE(ce_api_a0, filesystem);
    CE_UNLOAD_STATIC_MODULE(ce_api_a0, cdb);
    CE_UNLOAD_STATIC_MODULE(ce_api_a0, module);
    CE_UNLOAD_STATIC_MODULE(ce_api_a0, hashlib);
    // CE_UNLOAD_STATIC_MODULE(ce_api_a0, ydb);

    memsys_shutdown();
    api_shutdown();

    return true;
}
