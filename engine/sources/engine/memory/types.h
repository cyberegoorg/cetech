#ifndef CETECH_MEMSYS_TYPES_H
#define CETECH_MEMSYS_TYPES_H

#include "celib/memory/types.h"

struct MemSysApiV1 {
    struct cel_allocator *(*main_allocator)();
    struct cel_allocator *(*main_scratch_allocator)();
};

#endif //CETECH_MEMSYS_TYPES_H
