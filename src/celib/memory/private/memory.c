#include <memory.h>
#include <stdatomic.h>

#include <celib/memory/allocator.h>
#include <celib/api.h>

#include <celib/memory/memory.h>

#include "celib/memory/memory_tracer.h"

#define LOG_WHERE "memory"
#define CE_TRACE_MEMORY 0

#define _G ct_memory_global

#include "virt_alloc.inl"
#include "system_alloc.inl"

//static struct _G {
//} _G = {};

////

char *str_dup(const char *s,
              struct ce_alloc_t0 *allocator) {
    const uint32_t size = strlen(s) + 1;

    char *d = CE_ALLOC(allocator, char, size);
    CE_ASSERT("string", d != NULL);

    memset(d, '\0', sizeof(char) * size);
    strcpy(d, s);

    return d;
}

static struct ce_memory_a0 _api = {
        .system = &_system_allocator,
        .virt_system = &_virt_system_allocator,
        .str_dup = str_dup,
};

struct ce_memory_a0 *ce_memory_a0 = &_api;


void memory_tracer_init(struct ce_api_a0 *api);

void memory_tracer_shutdown();

void memory_register_api(struct ce_api_a0 *api) {
    memory_tracer_init(api);

    _system_allocator_inst.tracer = ce_memory_tracer_a0->create();

    api->add_api(CE_MEMORY_API, &_api, sizeof(_api));

}

void memory_init() {
}

void memsys_shutdown() {
    memory_tracer_shutdown();
}
