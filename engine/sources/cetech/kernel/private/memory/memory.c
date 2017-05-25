#include <memory.h>

#include <cetech/core/allocator.h>
#include <cetech/core/errors.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/api.h>

#include "_allocator.h"


#define LOG_WHERE "memory"


#define _G MemorySystemGlobals
struct G {
    struct allocator *default_allocator;
    struct allocator *default_scratch_allocator;
} MemorySystemGlobals = {0};

struct allocator *_memsys_main_allocator() {
    return _G.default_allocator;
}

struct allocator *_memsys_main_scratch_allocator() {
    return _G.default_scratch_allocator;
}

void memsys_init_api(struct api_v0* api) {
    static struct memory_api_v0 _api = {0};

    _api.main_allocator = _memsys_main_allocator;
    _api.main_scratch_allocator = _memsys_main_scratch_allocator;

    api->register_api("memory_api_v0", &_api);
}

static void _init( struct api_v0* api) {

}

static void _shutdown() {
}

void memsys_init(int scratch_buffer_size) {
    _G = (struct G) {0};

    _G.default_allocator = malloc_allocator_create();

    _G.default_scratch_allocator = scratch_allocator_create(
            _G.default_allocator,
            scratch_buffer_size);
}

void memsys_shutdown() {
    scratch_allocator_destroy(_G.default_scratch_allocator);
    malloc_allocator_destroy(_G.default_allocator);
}


void *memsys_get_module_api(int api) {
    switch (api) {
        case PLUGIN_EXPORT_API_ID: {
            static struct module_api_v0 module = {0};

            module.init = _init;
            module.shutdown = _shutdown;

            return &module;
        }

        default:
            return NULL;
    }
}

