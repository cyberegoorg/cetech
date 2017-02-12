#ifndef CELIB_MEMSYS_H
#define CELIB_MEMSYS_H

#include "celib/memory/types.h"

void memsys_init(int scratch_buffer_size);

void memsys_shutdown();

struct cel_allocator *_memsys_main_allocator();

struct cel_allocator *_memsys_main_scratch_allocator();

#include "celib/memory/types.h"

struct MemSysApiV1 {
    struct cel_allocator *(*main_allocator)();
    struct cel_allocator *(*main_scratch_allocator)();
};


#endif //CELIB_MEMSYS_H
