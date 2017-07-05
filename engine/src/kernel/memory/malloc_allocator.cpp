#ifndef CETECH_ALLOCATOR_MALLOC_H
#define CETECH_ALLOCATOR_MALLOC_H

#include <stdint.h>

#include <cetech/kernel/log.h>
#include <cetech/celib/allocator.h>
#include <cetech/kernel/errors.h>

#include "header.h"

#include "core_allocator_private.h"


#define MAX_MEM_TRACE 1024

static uint32_t size_with_padding(uint32_t size,
                                  uint32_t align) {
    return size + align + sizeof(struct Header);
}

struct malloc_allocator {
    uint32_t total_allocated;
    memory::allocator_trace_entry trace[MAX_MEM_TRACE];
};

void *malloc_allocator_allocate(allocator_instance_v0 *allocator,
                                void *ptr,
                                uint32_t size,
                                uint32_t align) {

    auto *core_alloc = core_allocator::get();
    auto *a = (struct malloc_allocator *) allocator;

    if (size) {
        const uint32_t ts = size_with_padding(size, align);
        Header *h = CETECH_ALLOCATE(core_alloc, Header, ts);

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

        memory::allocator_stop_trace_pointer(a->trace, MAX_MEM_TRACE, ptr);

        CETECH_FREE(core_alloc, h);

        return NULL;
    }
}

uint32_t malloc_allocator_allocated_size(void *p) {
    return header(p)->size;
}

uint32_t malloc_allocator_total_allocated(struct allocator *allocator) {
    struct malloc_allocator *a = (struct malloc_allocator *) allocator->inst;

    return a->total_allocated;
}

namespace memory {
    struct allocator *malloc_allocator_create() {
        auto *core_alloc = core_allocator::get();

        auto *a = CETECH_ALLOCATE(core_alloc, allocator, sizeof(allocator));

        struct malloc_allocator *m = CETECH_ALLOCATE(core_alloc,
                                                     malloc_allocator,
                                                     sizeof(malloc_allocator));
        m->total_allocated = 0;

        *a = (struct allocator) {
                .inst = m,
                .reallocate = malloc_allocator_allocate,
                .total_allocated = malloc_allocator_total_allocated,
                .allocated_size = malloc_allocator_allocated_size
        };

        return a;
    }

    void malloc_allocator_destroy(struct allocator *a) {
        auto *core_alloc = core_allocator::get();
        struct malloc_allocator *m = (struct malloc_allocator *) a->inst;

        memory::allocator_check_trace(m->trace, MAX_MEM_TRACE);

        CETECH_FREE(core_alloc, m);
        CETECH_FREE(core_alloc, a);
    }
}

#endif //CETECH_ALLOCATOR_MALLOC_H
