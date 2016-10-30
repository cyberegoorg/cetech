#ifndef CELIB_MEMSYS_H
#define CELIB_MEMSYS_H

#include "types.h"

void memsys_init(int scratch_buffer_size);

void memsys_shutdown();

struct allocator *memsys_main_allocator();

struct allocator *memsys_main_scratch_allocator();


#endif //CELIB_MEMSYS_H
