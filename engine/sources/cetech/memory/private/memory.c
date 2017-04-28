#include <cetech/memory/allocator.h>
#include <cetech/os/errors.h>

#include <cetech/module/module.h>
#include <cetech/memory/memory.h>
#include <memory.h>

#define LOG_WHERE "memory"


#define _G MemorySystemGlobals
struct G {
    struct cel_allocator *default_allocator;
    struct cel_allocator *default_scratch_allocator;
} MemorySystemGlobals = {0};



void *memory_copy(void *__restrict dest,
                  const void *__restrict src,
                  size_t n) {
    return memcpy(dest, src, n);
}

void *memory_set(void *__restrict dest,
                 int c,
                 size_t n) {
    return memset(dest, c, n);
}

const void *pointer_align_forward(const void *p,
                                  uint32_t align) {
    uintptr_t pi = (uintptr_t) p;
    const uint32_t mod = pi % align;
    if (mod)
        pi += (align - mod);
    return (void *) pi;
}

const void *pointer_add(const void *p,
                        uint32_t bytes) {
    return (const void *) ((const char *) p + bytes);
}


const void *pointer_sub(const void *p,
                        uint32_t bytes) {
    return (const void *) ((const char *) p - bytes);
}


static void _init(get_api_fce_t get_engine_api) {
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

struct cel_allocator *_memsys_main_allocator() {
    return _G.default_allocator;
}

struct cel_allocator *_memsys_main_scratch_allocator() {
    return _G.default_scratch_allocator;
}

void *memsys_get_module_api(int api,
                            int version) {
    switch (api) {
        case PLUGIN_EXPORT_API_ID:
            switch (version) {
                case 0: {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;

                    return &module;
                }

                default:
                    return NULL;
            };
        case MEMORY_API_ID:
            switch (version) {
                case 0: {
                    static struct MemSysApiV0 api = {0};

                    api.main_allocator = _memsys_main_allocator;
                    api.main_scratch_allocator = _memsys_main_scratch_allocator;

                    return &api;
                }

                default:
                    return NULL;
            };

        default:
            return NULL;
    }
}