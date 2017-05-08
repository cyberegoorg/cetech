#include <stdint.h>

#include <cetech/core/log.h>
#include <cetech/core/errors.h>
#include <cetech/core/allocator.h>

#define LOG_WHERE "allocator"

//void* allocator_allocate(struct allocator* allocator, uint32_t size, uint32_t align) {
//    return allocator->allocate(allocator, size, align);
//}
//
//void allocator_deallocate(struct allocator* allocator, void* p) {
//    allocator->deallocate(allocator, p);
//}
//
//uint32_t allocator_total_allocated(struct allocator* allocator){
//    return allocator->total_allocated(allocator);
//}
//
//uint32_t allocator_allocated_size(struct allocator* allocator, void* p){
//    return allocator->allocated_size(p);
//}

void *memory_malloc(size_t size) {
    void *mem = malloc(size);
    if (mem == NULL) {
        log_error("malloc", "Malloc return NULL");
        return NULL;
    }

    return mem;
}

void memory_free(void *ptr) {
    free(ptr);
}


void allocator_trace_pointer(struct allocator_trace_entry *entries,
                             uint64_t max_entries,
                             void *p) {
    char *stacktrace_str = stacktrace(3);

    for (int i = 0; i < max_entries; ++i) {
        if (!entries[i].used) {
            entries[i].used = 1;
            entries[i].ptr = p;
            entries[i].stacktrace = stacktrace_str;
            break;
        }
    }
}

void allocator_stop_trace_pointer(struct allocator_trace_entry *entries,
                                  uint64_t max_entries,
                                  void *p) {
    for (int i = 0; i < max_entries; ++i) {
        if (entries[i].ptr != p) {
            continue;
        }

        entries[i].used = 0;

        stacktrace_free(entries[i].stacktrace);
        entries[i].stacktrace = NULL;
    }
}

void allocator_check_trace(struct allocator_trace_entry *entries,
                           uint64_t max_entries) {
    for (int i = 0; i < max_entries; ++i) {
        if (!entries[i].used) {
            continue;
        }

        log_error(LOG_WHERE, "memory_leak: %p\n  stacktrace:\n%s\n",
                  entries[i].ptr, entries[i].stacktrace);

        //allocator_free(allocator, entries[i].ptr); // TODO: need this?

        stacktrace_free(entries[i].stacktrace);
    }
}