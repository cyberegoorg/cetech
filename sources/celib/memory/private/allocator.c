#include "../allocator.h"
#include "../ialocator.h"
#include "../../errors/errors.h"

#define LOG_WHERE "allocator"

#if defined(CETECH_DEBUG)
void _trace_pointer(Alloc_t allocator,
                    void *p) {

    struct iallocator *m = (struct iallocator *) allocator;
    struct trace_entry *mem_trace = m->trace;

    char *stacktrace_str = utils_stacktrace(3);

    for (int i = 0; i < CE_MAX_MEM_TRACE; ++i) {
        if (!mem_trace[i].used) {
            mem_trace[i].used = 1;
            mem_trace[i].ptr = p;
            mem_trace[i].stacktrace = stacktrace_str;
            break;
        }
    }
}

void _stop_trace_pointer(Alloc_t allocator, void *p) {
    struct iallocator *m = (struct iallocator *) allocator;

    struct trace_entry *mem_trace = m->trace;

    for (int i = 0; i < CE_MAX_MEM_TRACE; ++i) {
        if (mem_trace[i].ptr != p) {
            continue;
        }

        mem_trace[i].used = 0;

        utils_stacktrace_free(mem_trace[i].stacktrace);
        mem_trace[i].stacktrace = NULL;
    }
}
#endif

void *alloc_alloc(Alloc_t allocator, size_t size) {
    CE_ASSERT(LOG_WHERE, allocator != NULL);
    CE_ASSERT(LOG_WHERE, size > 0);

    struct iallocator *a = (struct iallocator *) allocator;

#if defined(CETECH_DEBUG)
    log_debug(LOG_WHERE, "[%s] Alloc %zu bytes", a->type_name, size);
#endif

    void* p = a->alloc(allocator, size);

#if defined(CETECH_DEBUG)
    _trace_pointer(allocator, p);
#endif
    return p;
}

void alloc_free(Alloc_t allocator, void *ptr) {
    CE_ASSERT(LOG_WHERE, allocator != NULL);

    struct iallocator *a = (struct iallocator *) allocator;

    a->free(allocator, ptr);

#if defined(CETECH_DEBUG)
    _stop_trace_pointer(allocator, ptr);
#endif
}

void alloc_destroy(Alloc_t allocator) {
#if defined(CETECH_DEBUG)
    struct iallocator *m = (struct iallocator *) allocator;

    int ok = 1;
    for (int i = 0; i < CE_MAX_MEM_TRACE; ++i) {
        if (!m->trace[i].used) {
            continue;
        }

        ok = 0;

        log_error(LOG_WHERE, "memory_leak: %p\n  stacktrace:\n%s\n",
                  m->trace[i].ptr, m->trace[i].stacktrace);

        alloc_free(allocator, m->trace[i].ptr);

        utils_stacktrace_free(m->trace[i].stacktrace);
    }

    CE_ASSERT(LOG_WHERE, ok);
#endif
}