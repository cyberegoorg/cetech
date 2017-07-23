#ifndef CETECH_ALLOCATOR_MALLOC_H
#define CETECH_ALLOCATOR_MALLOC_H

#include <stdint.h>

#include <cetech/kernel/log.h>
#include <cetech/celib/allocator.h>
#include <cetech/kernel/errors.h>

#include "allocator_core_private.h"
#include "memory_private.h"


#define MAX_MEM_TRACE 1024

static uint32_t size_with_padding(uint32_t size,
                                  uint32_t align) {
    return size + align + sizeof(struct Header);
}

struct allocator_malloc {
    uint32_t total_allocated;
    memory::allocator_trace_entry trace[MAX_MEM_TRACE];
};

void *malloc_allocator_allocate(cel_alloc_inst *allocator,
                                void *ptr,
                                uint32_t size,
                                uint32_t align) {

    auto *core_alloc = core_allocator::get();
    auto *a = (struct allocator_malloc *) allocator;

    if (size) {
        const uint32_t ts = size_with_padding(size, align);
        Header *h = CEL_ALLOCATE(core_alloc, Header, ts);

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

        CEL_FREE(core_alloc, h);

        return NULL;
    }
}

uint32_t malloc_allocator_allocated_size(void *p) {
    return header(p)->size;
}

uint32_t malloc_allocator_total_allocated(cel_alloc *allocator) {
    struct allocator_malloc *a = (struct allocator_malloc *) allocator->inst;

    return a->total_allocated;
}

namespace memory {
    cel_alloc *malloc_allocator_create() {
        auto *core_alloc = core_allocator::get();

        auto *a = CEL_ALLOCATE(core_alloc, cel_alloc, sizeof(cel_alloc));

        struct allocator_malloc *m = CEL_ALLOCATE(core_alloc,
                                                     allocator_malloc,
                                                     sizeof(allocator_malloc));
        m->total_allocated = 0;

        *a = (cel_alloc) {
                .inst = m,
                .reallocate = malloc_allocator_allocate,
                .total_allocated = malloc_allocator_total_allocated,
        };

        return a;
    }

    void malloc_allocator_destroy(cel_alloc *a) {
        auto *core_alloc = core_allocator::get();
        struct allocator_malloc *m = (struct allocator_malloc *) a->inst;

        memory::allocator_check_trace(m->trace, MAX_MEM_TRACE);

        CEL_FREE(core_alloc, m);
        CEL_FREE(core_alloc, a);
    }
}

#endif //CETECH_ALLOCATOR_MALLOC_H
