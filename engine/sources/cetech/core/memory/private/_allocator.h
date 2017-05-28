#ifndef _CETECH_ALLOCATOR_H
#define _CETECH_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>


struct allocator_trace_entry {
    void *ptr;
    char *stacktrace;
    char used;
};

//==============================================================================
// Malloc
//==============================================================================

void *memory_malloc(size_t size);

void memory_free(void *ptr);

//==============================================================================
// Allocator
//==============================================================================

void allocator_trace_pointer(struct allocator_trace_entry *entries,
                             uint64_t max_entries,
                             void *p);

void allocator_stop_trace_pointer(struct allocator_trace_entry *entries,
                                  uint64_t max_entries,
                                  void *p);

void allocator_check_trace(struct allocator_trace_entry *entries,
                           uint64_t max_entries);

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


#endif //_CETECH_ALLOCATOR_H
