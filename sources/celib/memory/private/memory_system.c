#include <stdint.h>
#include "celib/memory/memory.h"
#include "celib/errors/errors.h"


#include "allocator_malloc.h"
#include "allocator_scratch.h"

#define LOG_WHERE "memory_system"

////


///
struct G {
    struct allocator_malloc default_allocator;
    struct allocator_scratch default_scratch_allocator;
}   _G;

void memsys_init(int scratch_buffer_size) {
    _G = (struct G) {0};

    malloc_allocator_init(&_G.default_allocator);
    scratch_allocator_ctor(&_G.default_scratch_allocator, (struct allocator *) &_G.default_allocator,
                           scratch_buffer_size);
}

void memsys_shutdown() {
    scratch_allocator_dtor(&_G.default_scratch_allocator);
    malloc_allocator_dtor(&_G.default_allocator);
}

struct allocator *memsys_main_allocator() {
    return (struct allocator *) &_G.default_allocator;
}

struct allocator *memsys_main_scratch_allocator() {
    return (struct allocator *) &_G.default_scratch_allocator;
}
