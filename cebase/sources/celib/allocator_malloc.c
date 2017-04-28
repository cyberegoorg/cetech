//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CELIB_ALLOCATOR_MALLOC_H
#define CELIB_ALLOCATOR_MALLOC_H

#include <stdint.h>
#include <celib/log.h>

#include "celib/memory.h"
#include "celib/errors.h"
#include "celib/allocator.h"
#include "header.h"



#define MAX_MEM_TRACE 1024

static uint32_t size_with_padding(uint32_t size,
                                  uint32_t align) {
    return size + align + sizeof(struct Header);
}

struct cel_allocator_malloc {
    struct cel_allocator base;
    uint32_t total_allocated;
    struct cel_allocator_trace_entry trace[MAX_MEM_TRACE];
};

void *malloc_allocator_allocate(struct cel_allocator *allocator,
                                uint32_t size,
                                uint32_t align) {
    struct cel_allocator_malloc *a = (struct cel_allocator_malloc *) allocator;

    const uint32_t ts = size_with_padding(size, align);
    struct Header *h = (struct Header *) cel_malloc(ts);

    void *p = data_pointer(h, align);
    fill(h, p, ts);
    a->total_allocated += ts;

    allocator_trace_pointer(a->trace, MAX_MEM_TRACE, p);

    return p;
}

void malloc_allocator_deallocate(struct cel_allocator *allocator,
                                 void *p) {
    struct cel_allocator_malloc *a = (struct cel_allocator_malloc *) allocator;

    if (!p)
        return;

    struct Header *h = header(p);
    a->total_allocated -= h->size;

    allocator_stop_trace_pointer(a->trace, MAX_MEM_TRACE, p);

    cel_free(h);
}

uint32_t malloc_allocator_allocated_size(void *p) {
    return header(p)->size;
}

uint32_t malloc_allocator_total_allocated(struct cel_allocator *allocator) {
    struct cel_allocator_malloc *a = (struct cel_allocator_malloc *) allocator;

    return a->total_allocated;
}

struct cel_allocator *malloc_allocator_create() {
    struct cel_allocator_malloc *m = cel_malloc(sizeof(struct cel_allocator_malloc));

    m->base = (struct cel_allocator) {
            .allocate = malloc_allocator_allocate,
            .deallocate = malloc_allocator_deallocate,
            .total_allocated = malloc_allocator_total_allocated,
            .allocated_size = malloc_allocator_allocated_size
    };

    m->total_allocated = 0;

    return (struct cel_allocator *) m;
}

void malloc_allocator_destroy(struct cel_allocator *a) {
    struct cel_allocator_malloc *m = (struct cel_allocator_malloc *) a;

    allocator_check_trace(m->trace, MAX_MEM_TRACE);

    //CEL_ASSERT_MSG("memory.malloc", m->total_allocated == 0, "%d bytes is not deallocate", m->total_allocated);
    cel_free(m);
}

#endif //CELIB_ALLOCATOR_MALLOC_H
