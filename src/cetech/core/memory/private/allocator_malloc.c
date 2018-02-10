#ifndef CETECH_ALLOCATOR_MALLOC_H
#define CETECH_ALLOCATOR_MALLOC_H

#include <stdint.h>

#include "cetech/core/memory/allocator.h"
#include "allocator_core_private.h"
#include "memory_private.h"

static uint32_t size_with_padding(uint32_t size,
                                  uint32_t align) {
    return size + align + sizeof(struct Header);
}

struct allocator_malloc {
    uint32_t total_allocated;
};

void *malloc_allocator_allocate(const struct ct_alloc *allocator,
                                void *ptr,
                                uint32_t size,
                                uint32_t align) {

    struct ct_alloc *core_alloc = coreallocator_get();
    struct allocator_malloc *a = (struct allocator_malloc *) allocator->inst;

    if (size) {
        const uint32_t ts = size_with_padding(size, align);
        struct Header *h = CT_ALLOC(core_alloc, struct Header, ts);

        void *p = data_pointer(h, align);
        fill(h, p, ts);
        a->total_allocated += ts;

        return p;
    } else {
        if (!ptr) {
            return NULL;
        }

        struct Header *h = header(ptr);
        a->total_allocated -= h->size;

        CT_FREE(core_alloc, h);

        return NULL;
    }
}

uint32_t malloc_allocator_total_allocated(struct ct_alloc *allocator) {
    struct allocator_malloc *a = (struct allocator_malloc *) allocator->inst;

    return a->total_allocated;
}

static struct ct_alloc_fce alloc_fce = {
        .reallocate = malloc_allocator_allocate,
        .total_allocated = malloc_allocator_total_allocated,
};

struct ct_alloc *malloc_allocator_create() {
    struct ct_alloc *core_alloc = coreallocator_get();

    struct ct_alloc *a = CT_ALLOC(core_alloc, struct ct_alloc,
                                  sizeof(struct ct_alloc));

    struct allocator_malloc *m = CT_ALLOC(core_alloc,
                                          struct allocator_malloc,
                                          sizeof(struct allocator_malloc));
    m->total_allocated = 0;

    *a = (struct ct_alloc) {
            .inst = m,
            .call = &alloc_fce,
    };

    return a;
}

void malloc_allocator_destroy(struct ct_alloc *a) {
    struct ct_alloc *core_alloc = coreallocator_get();
    struct allocator_malloc *m = (struct allocator_malloc *) a->inst;

    CT_FREE(core_alloc, m);
    CT_FREE(core_alloc, a);
}

#endif //CETECH_ALLOCATOR_MALLOC_H
