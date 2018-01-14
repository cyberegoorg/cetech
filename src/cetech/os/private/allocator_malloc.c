#ifndef CETECH_ALLOCATOR_MALLOC_H
#define CETECH_ALLOCATOR_MALLOC_H

#include <stdint.h>

#include "celib/allocator.h"
#include "allocator_core_private.h"
#include "memory_private.h"


#define MAX_MEM_TRACE 1024

static uint32_t size_with_padding(uint32_t size,
                                  uint32_t align) {
    return size + align + sizeof(struct Header);
}

struct allocator_malloc {
    uint32_t total_allocated;
    struct allocator_trace_entry trace[MAX_MEM_TRACE];
};

void *malloc_allocator_allocate(const struct cel_alloc *allocator,
                                void *ptr,
                                uint32_t size,
                                uint32_t align) {

    struct cel_alloc *core_alloc = coreallocator_get();
    struct allocator_malloc *a = (struct allocator_malloc *) allocator->inst;

    if (size) {
        const uint32_t ts = size_with_padding(size, align);
        struct Header *h = CEL_ALLOCATE(core_alloc, struct Header, ts);

        void *p = data_pointer(h, align);
        fill(h, p, ts);
        a->total_allocated += ts;

        allocator_trace_pointer(a->trace, MAX_MEM_TRACE, p);

        return p;
    } else {
        if (!ptr) {
            return NULL;
        }

        struct Header *h = header(ptr);
        a->total_allocated -= h->size;

        allocator_stop_trace_pointer(a->trace, MAX_MEM_TRACE, ptr);

        CEL_FREE(core_alloc, h);

        return NULL;
    }
}

uint32_t malloc_allocator_allocated_size(void *p) {
    return header(p)->size;
}

uint32_t malloc_allocator_total_allocated(struct cel_alloc *allocator) {
    struct allocator_malloc *a = (struct allocator_malloc *) allocator->inst;

    return a->total_allocated;
}

static struct cel_alloc_fce alloc_fce = {
        .reallocate = malloc_allocator_allocate,
        .total_allocated = malloc_allocator_total_allocated,
};

struct cel_alloc *malloc_allocator_create() {
    struct cel_alloc *core_alloc = coreallocator_get();

    struct cel_alloc *a = CEL_ALLOCATE(core_alloc, struct cel_alloc, sizeof(struct cel_alloc));

    struct allocator_malloc *m = CEL_ALLOCATE(core_alloc,
                                              struct allocator_malloc,
                                              sizeof(struct allocator_malloc));
    m->total_allocated = 0;

    *a = (struct cel_alloc) {
            .inst = m,
            .call = &alloc_fce,
    };

    return a;
}

void malloc_allocator_destroy(struct cel_alloc *a) {
    struct cel_alloc *core_alloc = coreallocator_get();
    struct allocator_malloc *m = (struct allocator_malloc *) a->inst;

    allocator_check_trace(m->trace, MAX_MEM_TRACE);

    CEL_FREE(core_alloc, m);
    CEL_FREE(core_alloc, a);
}

#endif //CETECH_ALLOCATOR_MALLOC_H
