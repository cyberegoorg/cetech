#ifndef CELIB_ALLOCATOR_H
#define CELIB_ALLOCATOR_H

#include <memory.h>

#include "types.h"

//==============================================================================
// Defines
//==============================================================================

#define CEL_SIZE_NOT_TRACKED 0xffffffffu

#define allocator_allocate(a, s, align) (a)->allocate(a, s, align)
#define allocator_deallocate(a, p) (a)->deallocate(a, p)
#define allocator_total_allocated(a) (a)->total_allocated(a)
#define allocator_allocated_size(a, p) (a)->allocated_size(a, p)

#define CEL_ALLOCATE(a, T, size) (T*) allocator_allocate((a), sizeof(T) * size, CEL_ALIGNOF(T))
#define CEL_ALLOCATE_ALIGN(a, T, size, align) (T*) allocator_allocate((a), size, align)
#define CEL_DEALLOCATE(a, p) allocator_deallocate((a), p)

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

struct cel_allocator {
    void *(*allocate)(struct cel_allocator *allocator,
                      uint32_t size,
                      uint32_t align);

    void (*deallocate)(struct cel_allocator *allocator,
                       void *p);

    uint32_t (*total_allocated)(struct cel_allocator *allocator);

    uint32_t (*allocated_size)(void *p);
};

struct cel_allocator_trace_entry {
    void *ptr;
    char *stacktrace;
    char used;
};




//==============================================================================
// Allocator
//==============================================================================

void allocator_trace_pointer(struct cel_allocator_trace_entry *entries,
                             u64 max_entries,
                             void *p);

void allocator_stop_trace_pointer(struct cel_allocator_trace_entry *entries,
                                  u64 max_entries,
                                  void *p);

void allocator_check_trace(struct cel_allocator_trace_entry *entries,
                           u64 max_entries);

//==============================================================================
// Malloc allocator
//==============================================================================


struct cel_allocator *malloc_allocator_create();

void malloc_allocator_destroy(struct cel_allocator *a);


//==============================================================================
// Scratch allocator
//==============================================================================

struct cel_allocator *scratch_allocator_create(struct cel_allocator *backing,
                                               int size);

void scratch_allocator_destroy(struct cel_allocator *a);

#endif //CELIB_ALLOCATOR_H
