#ifndef CELIB_ALLOCATOR_H
#define CELIB_ALLOCATOR_H

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
// Malloc/Free
//==============================================================================

static void *cel_malloc(size_t size) {
    void *mem = malloc(size);
    if (mem == NULL) {
        log_error("malloc", "Malloc return NULL");
        return NULL;
    }

    return mem;
}

static void cel_free(void *ptr) {
    free(ptr);
}

//==============================================================================
// Allocator
//==============================================================================

void allocator_trace_pointer(struct allocator_trace_entry *entries,
                             u64 max_entries,
                             void *p);

void allocator_stop_trace_pointer(struct allocator_trace_entry *entries,
                                  u64 max_entries,
                                  void *p);

void allocator_check_trace(struct allocator_trace_entry *entries,
                           u64 max_entries);

//==============================================================================
// Malloc allocator
//==============================================================================


struct allocator *malloc_allocator_create();

void malloc_allocator_destroy(struct allocator *a);


//==============================================================================
// Scratch allocator
//==============================================================================

struct allocator *scratch_allocator_create(struct allocator *backing,
                                           int size);

void scratch_allocator_destroy(struct allocator *a);

#endif //CELIB_ALLOCATOR_H
