#include <celib/memory/memory.h>
#include <celib/errors/errors.h>

#define LOG_WHERE "memory"


#define _G MemorySystemGlobals
struct G {
    struct cel_allocator *default_allocator;
    struct cel_allocator *default_scratch_allocator;
} MemorySystemGlobals = {0};

void memsys_init(int scratch_buffer_size) {
    _G = (struct G) {0};

    _G.default_allocator = malloc_allocator_create();

    _G.default_scratch_allocator = scratch_allocator_create(_G.default_allocator,
                                                            scratch_buffer_size);
}

void memsys_shutdown() {
    scratch_allocator_destroy(_G.default_scratch_allocator);
    malloc_allocator_destroy(_G.default_allocator);
}

struct cel_allocator *memsys_main_allocator() {
    return _G.default_allocator;
}

struct cel_allocator *memsys_main_scratch_allocator() {
    return _G.default_scratch_allocator;
}
