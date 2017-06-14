//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CETECH_ALLOCATOR_SCRATCH_H
#define CETECH_ALLOCATOR_SCRATCH_H


#include <cetech/core/errors.h>
#include <cetech/celib/allocator.h>

#include "header.h"

struct allocator_scratch {
    struct allocator base;
    struct allocator *backing;
    char *begin;
    char *end;
    char *allocate;
    char *free;
};

int in_use(struct allocator_scratch *a,
           void *p) {
    if (a->free == a->allocate)
        return 0;

    if (a->allocate > a->free)
        return ((char *) p >= a->free) && ((char *) p < a->allocate);

    return ((char *) p >= a->free) || ((char *) p < a->allocate);
}

void *scratch_allocator_allocate(struct allocator *allocator,
                                 uint32_t size,
                                 uint32_t align) {
    struct allocator_scratch *a = (struct allocator_scratch *) allocator;

    //CE_ASSERT("scratch", align % 4 == 0);
    size = ((size + 3) / 4) * 4;

    char *p = a->allocate;
    struct Header *h = (struct Header *) p;
    char *data = (char *) data_pointer(h, align);
    p = data + size;

    // Reached the end of the buffer, wrap around to the beginning.
    if (p > a->end) {
        h->size = (a->end - (char *) h) | 0x80000000u;

        p = a->begin;
        h = (struct Header *) p;
        data = (char *) data_pointer(h, align);
        p = data + size;
    }

    // If the buffer is exhausted use the backing allocator instead.
    if (in_use(a, p))
        return CETECH_ALLOCATE_ALIGN(a->backing, void, size, align);

    fill(h, data, p - (char *) h);
    a->allocate = p;
    return data;
}

void scratch_allocator_deallocate(struct allocator *allocator,
                                  void *p) {
    struct allocator_scratch *a = (struct allocator_scratch *) allocator;

    if (!p)
        return;

    if ((char *) p < a->begin || (char *) p >= a->end) {
        allocator_deallocate(a->backing, p);
        return;
    }

    // Mark this slot as free
    struct Header *h = header(p);
    CETECH_ASSERT("scratch", (h->size & 0x80000000u) == 0);
    h->size = h->size | 0x80000000u;

    // Advance the free pointer past all free slots.
    while (a->free != a->allocate) {
        struct Header *h = (struct Header *) a->free;
        if ((h->size & 0x80000000u) == 0)
            break;

        a->free += h->size & 0x7fffffffu;
        if (a->free == a->end)
            a->free = a->begin;
    }
}

uint32_t scratch_allocator_allocated_size(void *p) {
    struct Header *h = header(p);
    return h->size - ((char *) p - (char *) h);
}

uint32_t scratch_allocator_total_allocated(struct allocator *allocator) {
    struct allocator_scratch *a = (struct allocator_scratch *) allocator;

    return a->end - a->begin;

}

struct allocator *scratch_allocator_create(struct allocator *backing,
                                           int size) {
    struct allocator_scratch *m = (allocator_scratch *) memory::malloc(
            sizeof(struct allocator_scratch));

    m->base = (struct allocator) {
            .allocate = scratch_allocator_allocate,
            .deallocate = scratch_allocator_deallocate,
            .total_allocated = scratch_allocator_total_allocated,
            .allocated_size = scratch_allocator_allocated_size
    };

    m->backing = backing;
    m->begin = CETECH_ALLOCATE(backing, char, size);
    m->end = m->begin + size;
    m->allocate = m->begin;
    m->free = m->begin;

    return (struct allocator *) m;
}

void scratch_allocator_destroy(struct allocator *a) {
    struct allocator_scratch *m = (struct allocator_scratch *) a;

    allocator_deallocate(m->backing, m->begin);
}

#endif // CETECH_ALLOCATOR_SCRATCH_H
