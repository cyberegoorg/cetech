#ifndef CETECH_IALOCATOR_H
#define CETECH_IALOCATOR_H

#include "allocator.h"

#define CE_MAX_MEM_TRACE 1024

struct iallocator {
    void *(*alloc)(Alloc_t allocator, size_t size);

    void (*free)(Alloc_t allocator, void *ptr);

    const char *type_name;

#if defined(CETECH_DEBUG)
    struct trace_entry {
        void *ptr;
        char *stacktrace;
        char used;
    } trace[CE_MAX_MEM_TRACE];
#endif
};


#endif //CETECH_IALOCATOR_H
