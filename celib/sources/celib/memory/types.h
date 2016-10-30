#ifndef CELIB_MEMORY_TYPES_H
#define CELIB_MEMORY_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Defines
//==============================================================================

#define _4B  (  4 )
#define _8B  (  8 )
#define _16B ( 16 )
#define _32B ( 32 )
#define _64B ( 64 )

#define _1KiB  (  1024    )
#define _4KiB  ( _1KiB*4  )
#define _8KiB  ( _1KiB*8  )
#define _16KiB ( _1KiB*16 )
#define _32KiB ( _1KiB*32 )

//==============================================================================
// Allocator
//==============================================================================

struct allocator {
    void *(*allocate)(struct allocator *allocator,
                      uint32_t size,
                      uint32_t align);

    void (*deallocate)(struct allocator *allocator,
                       void *p);

    uint32_t (*total_allocated)(struct allocator *allocator);

    uint32_t (*allocated_size)(void *p);
};

struct allocator_trace_entry {
    void *ptr;
    char *stacktrace;
    char used;
};


#endif //CELIB_MEMORY_TYPES_H
