#ifndef CETECH_MEMORY_SYSTEM_H
#define CETECH_MEMORY_SYSTEM_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>

#include "types.h"
#include "celib/types.h"

//==============================================================================
// Includes
//==============================================================================


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
// Memory system
//==============================================================================

void memsys_init(int scratch_buffer_size);

void memsys_shutdown();

struct allocator *memsys_main_allocator();

struct allocator *memsys_main_scratch_allocator();

//==============================================================================
// Allocator
//==============================================================================

#define CE_SIZE_NOT_TRACKED 0xffffffffu

#define allocator_allocate(a, s, align) (a)->allocate(a, s, align)
#define allocator_deallocate(a, p) (a)->deallocate(a, p)
#define allocator_total_allocated(a) (a)->total_allocated(a)
#define allocator_allocated_size(a, p) (a)->allocated_size(a, p)

#define CE_ALLOCATE(a, T, size) (T*) allocator_allocate((a), sizeof(T) * size, CE_ALIGNOF(T))
#define CE_ALLOCATE_ALIGN(a, T, size, align) (T*) allocator_allocate((a), size, align)
#define CE_DEALLOCATE(a, p) allocator_deallocate((a), p)

void allocator_trace_pointer(struct allocator_trace_entry *entries, u64 max_entries, void *p);

void allocator_stop_trace_pointer(struct allocator_trace_entry *entries, u64 max_entries, void *p);

void allocator_check_trace(struct allocator_trace_entry *entries, u64 max_entries);

#endif //CETECH_MEMORY_SYSTEM_H
