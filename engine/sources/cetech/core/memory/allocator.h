#ifndef CETECH_ALLOCATOR_H
#define CETECH_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

//==============================================================================
// Defines
//==============================================================================

#define CETECH_SIZE_NOT_TRACKED 0xffffffffu

#define allocator_allocate(a, s, align) (a)->allocate(a, s, align)
#define allocator_deallocate(a, p) (a)->deallocate(a, p)
#define allocator_total_allocated(a) (a)->total_allocated(a)
#define allocator_allocated_size(a, p) (a)->allocated_size(a, p)

#define CETECH_ALLOCATE(a, T, size) (T*) allocator_allocate((a), sizeof(T) * size, CETECH_ALIGNOF(T))
#define CETECH_ALLOCATE_ALIGN(a, T, size, align) (T*) allocator_allocate((a), size, align)
#define CETECH_DEALLOCATE(a, p) allocator_deallocate((a), p)

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


#endif //CETECH_ALLOCATOR_H
