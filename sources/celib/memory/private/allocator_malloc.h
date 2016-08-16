#ifndef CETECH_ALLOCATOR_MALLOC_H
#define CETECH_ALLOCATOR_MALLOC_H

#include <stdint.h>
#include <celib/memory/memory.h>
#include <celib/errors/errors.h>
#include <celib/os/memory.h>
#include "header.h"

#define MAX_MEM_TRACE 1024

static uint32_t size_with_padding(uint32_t size, uint32_t align) {
    return size + align + sizeof(struct Header);
}

struct allocator_malloc {
    struct allocator base;
    uint32_t total_allocated;
    struct allocator_trace_entry trace[MAX_MEM_TRACE];
};


void *malloc_allocator_allocate(struct allocator *allocator, uint32_t size, uint32_t align) {
    struct allocator_malloc *a = (struct allocator_malloc *) allocator;

    const uint32_t ts = size_with_padding(size, align);
    struct Header *h = (struct Header *) os_malloc(ts);

    void *p = data_pointer(h, align);
    fill(h, p, ts);
    a->total_allocated += ts;

    allocator_trace_pointer(a->trace, MAX_MEM_TRACE, p);

    return p;
}

void malloc_allocator_deallocate(struct allocator *allocator, void *p) {
    struct allocator_malloc *a = (struct allocator_malloc *) allocator;

    if (!p)
        return;

    struct Header *h = header(p);
    a->total_allocated -= h->size;

    allocator_stop_trace_pointer(a->trace, MAX_MEM_TRACE, p);

    os_free(h);
}

uint32_t malloc_allocator_allocated_size(void *p) {
    return header(p)->size;
}

uint32_t malloc_allocator_total_allocated(struct allocator *allocator) {
    struct allocator_malloc *a = (struct allocator_malloc *) allocator;

    return a->total_allocated;
}

void malloc_allocator_init(struct allocator_malloc *a) {
    a->base = (struct allocator) {
            .allocate = malloc_allocator_allocate,
            .deallocate = malloc_allocator_deallocate,
            .total_allocated = malloc_allocator_total_allocated,
            .allocated_size = malloc_allocator_allocated_size
    };

    a->total_allocated = 0;
}

void malloc_allocator_dtor(struct allocator_malloc *a) {
    allocator_check_trace(a->trace, MAX_MEM_TRACE);
    CE_ASSERT_MSG("memory.malloc", a->total_allocated == 0, "%d bytes is not deallocate", a->total_allocated);
}

#endif //CETECH_ALLOCATOR_MALLOC_H
