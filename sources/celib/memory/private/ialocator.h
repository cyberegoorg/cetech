#ifndef CETECH_IALOCATOR_H
#define CETECH_IALOCATOR_H

#include "../allocator.h"

#if defined(CETECH_DEBUG)
enum {
    MAX_MEM_TRACE = 128
};


struct trace_entry {
    void *ptr;
    char *stacktrace;
    char used;
};
#endif

struct iallocator {
    void *(*alloc)(Alloc_t allocator, size_t size);
    void (*free)(Alloc_t allocator, void *ptr);

#if defined(CETECH_DEBUG)
    const char* type_name;
    struct trace_entry trace[MAX_MEM_TRACE];
#endif

};


#endif //CETECH_IALOCATOR_H
