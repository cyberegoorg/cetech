//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CELIB_ALLOCATOR_SCRATCH_H
#define CELIB_ALLOCATOR_SCRATCH_H

#include "../memory.h"
#include "../../errors/errors.h"

#include "header.h"

struct cel_allocator_scratch {
    struct cel_allocator base;
    struct cel_allocator *backing;
    char *begin;
    char *end;
    char *allocate;
    char *free;
};

int in_use(struct cel_allocator_scratch *a,
           void *p) {
    if (a->free == a->allocate)
        return 0;

    if (a->allocate > a->free)
        return ((char *) p >= a->free) && ((char *) p < a->allocate);

    return ((char *) p >= a->free) || ((char *) p < a->allocate);
}

void *scratch_allocator_allocate(struct cel_allocator *allocator,
                                 uint32_t size,
                                 uint32_t align) {
    struct cel_allocator_scratch *a = (struct cel_allocator_scratch *) allocator;

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
        return CEL_ALLOCATE_ALIGN(a->backing, void, size, align);

    fill(h, data, p - (char *) h);
    a->allocate = p;
    return data;
}

void scratch_allocator_deallocate(struct cel_allocator *allocator,
                                  void *p) {
    struct cel_allocator_scratch *a = (struct cel_allocator_scratch *) allocator;

    if (!p)
        return;

    if ((char *) p < a->begin || (char *) p >= a->end) {
        allocator_deallocate(a->backing, p);
        return;
    }

    // Mark this slot as free
    struct Header *h = header(p);
    CEL_ASSERT("scratch", (h->size & 0x80000000u) == 0);
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

uint32_t scratch_allocator_total_allocated(struct cel_allocator *allocator) {
    struct cel_allocator_scratch *a = (struct cel_allocator_scratch *) allocator;

    return a->end - a->begin;

}

struct cel_allocator *scratch_allocator_create(struct cel_allocator *backing,
                                           int size) {
    struct cel_allocator_scratch *m = cel_malloc(sizeof(struct cel_allocator_scratch));

    m->base = (struct cel_allocator) {
            .allocate = scratch_allocator_allocate,
            .deallocate = scratch_allocator_deallocate,
            .total_allocated = scratch_allocator_total_allocated,
            .allocated_size = scratch_allocator_allocated_size
    };

    m->backing = backing;
    m->begin = CEL_ALLOCATE(backing, char, size);
    m->end = m->begin + size;
    m->allocate = m->begin;
    m->free = m->begin;

    return (struct cel_allocator *) m;
}

void scratch_allocator_destroy(struct cel_allocator *a) {
    struct cel_allocator_scratch *m = (struct cel_allocator_scratch *) a;

    allocator_deallocate(m->backing, m->begin);
}

#endif // CELIB_ALLOCATOR_SCRATCH_H
