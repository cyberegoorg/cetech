#ifndef CETECH_MEMORY_SYSTEM_H
#define CETECH_MEMORY_SYSTEM_H

#include "celib/memory/types.h"

//==============================================================================
// Interface
//==============================================================================

void memsys_init(int scratch_buffer_size);

void memsys_shutdown();

struct allocator *memsys_main_allocator();

struct allocator *memsys_main_scratch_allocator();


#endif //CETECH_MEMORY_SYSTEM_H
