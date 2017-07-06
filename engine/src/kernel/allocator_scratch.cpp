//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CETECH_ALLOCATOR_SCRATCH_H
#define CETECH_ALLOCATOR_SCRATCH_H


#include <cetech/kernel/errors.h>
#include <cetech/celib/allocator.h>
#include <cetech/kernel/macros.h>

#include "memory_private.h"
#include "allocator_core_private.h"

struct allocator_scratch {
    struct ct_allocator *backing;
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

void *scratch_allocator_allocate(allocator_instance_v0 *allocator,
                                 void *ptr,
                                 uint32_t size,
                                 uint32_t align) {
    struct allocator_scratch *a = (struct allocator_scratch *) allocator;

    if (size) {
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

        // If the buffer is exhausted use the backing ct_allocator instead.
        if (in_use(a, p))
            return CETECH_ALLOCATE_ALIGN(a->backing, void, size, align);

        fill(h, data, p - (char *) h);
        a->allocate = p;

        return data;
    } else {
        if (!ptr) {
            return NULL;
        }

        if ((char *) ptr < a->begin || (char *) ptr >= a->end) {
            CETECH_FREE(a->backing, ptr);
            return NULL;
        }

        // Mark this slot as free
        struct Header *h = header(ptr);
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
        return NULL;
    }
}

uint32_t scratch_allocator_allocated_size(void *p) {
    struct Header *h = header(p);
    return h->size - ((char *) p - (char *) h);
}

uint32_t scratch_allocator_total_allocated(struct ct_allocator *allocator) {
    struct allocator_scratch *a = (struct allocator_scratch *) allocator->inst;

    return a->end - a->begin;

}

namespace memory {
    struct ct_allocator *scratch_allocator_create(struct ct_allocator *backing,
                                               int size) {
        auto *core_alloc = core_allocator::get();
        auto *a = CETECH_ALLOCATE(core_alloc, ct_allocator, sizeof(ct_allocator));

        allocator_scratch *m = CETECH_ALLOCATE(core_alloc, allocator_scratch,
                                               sizeof(allocator_scratch));

        m->backing = backing;
        m->begin = CETECH_ALLOCATE(backing, char, size);
        m->end = m->begin + size;
        m->allocate = m->begin;
        m->free = m->begin;

        *a = (struct ct_allocator) {
                .inst = m,
                .reallocate = scratch_allocator_allocate,
                .total_allocated = scratch_allocator_total_allocated,
                .allocated_size = scratch_allocator_allocated_size
        };


        return a;
    }

    void scratch_allocator_destroy(struct ct_allocator *a) {
        auto *core_alloc = core_allocator::get();
        struct allocator_scratch *m = (struct allocator_scratch *) a->inst;

        CETECH_FREE(m->backing, m->begin);
        CETECH_FREE(core_alloc, m);
        CETECH_FREE(core_alloc, a);
    }
}

#endif // CETECH_ALLOCATOR_SCRATCH_H
