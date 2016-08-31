//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CETECH_MEMORY_H
#define CETECH_MEMORY_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>

#include "types.h"
#include "celib/types.h"

//==============================================================================
// Defines
//==============================================================================

#define CE_SIZE_NOT_TRACKED 0xffffffffu

#define allocator_allocate(a, s, align) (a)->allocate(a, s, align)
#define allocator_deallocate(a, p) (a)->deallocate(a, p)
#define allocator_total_allocated(a) (a)->total_allocated(a)
#define allocator_allocated_size(a, p) (a)->allocated_size(a, p)

#define CE_ALLOCATE(a, T, size) (T*) allocator_allocate((a), sizeof(T) * size, CE_ALIGNOF(T))
#define CE_ALLOCATE_ALIGN(a, T, size, align) (T*) allocator_allocate((a), size, align)
#define CE_DEALLOCATE(a, p) allocator_deallocate((a), p)

//==============================================================================
// Memory
//==============================================================================

const void *pointer_align_forward(const void *p, uint32_t align);

const void *pointer_add(const void *p, uint32_t bytes);

const void *pointer_sub(const void *p, uint32_t bytes);

void *memory_copy(void *__restrict dest, const void *__restrict src, size_t n);

void *memory_set(void *__restrict dest, int c, size_t n);

void *os_malloc(size_t size);

void os_free(void *ptr);


//==============================================================================
// Allocator
//==============================================================================

void allocator_trace_pointer(struct allocator_trace_entry *entries, u64 max_entries, void *p);

void allocator_stop_trace_pointer(struct allocator_trace_entry *entries, u64 max_entries, void *p);

void allocator_check_trace(struct allocator_trace_entry *entries, u64 max_entries);

//==============================================================================
// Malloc allocator
//==============================================================================


struct allocator *malloc_allocator_create();

void malloc_allocator_destroy(struct allocator *a);


//==============================================================================
// Scratch allocator
//==============================================================================

struct allocator *scratch_allocator_create(struct allocator *backing, int size);

void scratch_allocator_destroy(struct allocator *a);

#endif //CETECH_MEMORY_H
