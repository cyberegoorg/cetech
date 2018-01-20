#include <memory.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/os/errors.h>
#include <cetech/core/memory/memory.h>

#include "cetech/core/memory/allocator.h"
#include "cetech/core/log/log.h"

#include "memory_private.h"
#include "cetech/core/memory/private/allocator_core_private.h"

CETECH_DECL_API(ct_log_a0);

#define LOG_WHERE "memory"
#define ALLOCATOR_WHERE "allocator"

struct MemorySystemGlobals {
    struct ct_alloc *default_allocator;
    struct ct_alloc *default_scratch_allocator;
} _G = {};


extern char *stacktrace(int);

extern void stacktrace_free(char *);

void *data_pointer(struct Header *header,
                   uint32_t align) {
    const void *p = header + 1;
    return (void *) pointer_align_forward(p, align);
}

struct Header *header(void *data) {
    uint32_t *p = (uint32_t *) data;

    while (p[-1] == HEADER_PAD_VALUE)
        --p;

    return (struct Header *) p - 1;
}

void fill(struct Header *header,
          void *data,
          uint32_t size) {
    header->size = size;
    uint32_t *p = (uint32_t *) (header + 1);
    while (p < (uint32_t *) data)
        *p++ = HEADER_PAD_VALUE;
}

const void *pointer_align_forward(const void *p,
                                  uint32_t align) {
    uintptr_t pi = (uintptr_t) p;
    const uint32_t mod = pi % align;
    if (mod)
        pi += (align - mod);
    return (void *) pi;
}

void allocator_trace_pointer(struct allocator_trace_entry *entries,
                             uint32_t max_entries,
                             void *p) {
    //char *stacktrace_str = stacktrace(3);

    for (uint32_t i = 0; i < max_entries; ++i) {
        if (!entries[i].used) {
            entries[i].used = 1;
            entries[i].ptr = p;
            //entries[i].stacktrace = stacktrace_str;
            break;
        }
    }
}

void allocator_stop_trace_pointer(struct allocator_trace_entry *entries,
                                  uint32_t max_entries,
                                  void *p) {
    for (uint32_t i = 0; i < max_entries; ++i) {
        if (entries[i].ptr != p) {
            continue;
        }

        entries[i].used = 0;

        //stacktrace_free(entries[i].stacktrace);
        //entries[i].stacktrace = NULL;
    }
}

void allocator_check_trace(struct allocator_trace_entry *entries,
                           uint32_t max_entries) {
    for (uint32_t i = 0; i < max_entries; ++i) {
        if (!entries[i].used) {
            continue;
        }

        ct_log_a0.error(ALLOCATOR_WHERE,
                        "memory_leak: %p\n",
                        entries[i].ptr);

        //allocator_free(ct_alloc, entries[i].ptr); // TODO: need this?

        //stacktrace_free(entries[i].stacktrace);
    }
}

struct ct_alloc *memsys_main_allocator() {
    return _G.default_allocator;
}

char *str_dup(const char *s,
              struct ct_alloc *allocator) {
    const uint32_t size = strlen(s) + 1;

    char *d = CT_ALLOC(allocator, char, size);
    CETECH_ASSERT("string", d != NULL);

    memset(d, '\0', sizeof(char) * size);
    strcpy(d, s);

    return d;
}

void register_api(struct ct_api_a0 *api) {
    static struct ct_memory_a0 _api = {};

    _api.main_allocator = memsys_main_allocator;
    _api.str_dup = str_dup;

    api->register_api("ct_memory_a0", &_api);

    CETECH_GET_API(api, ct_log_a0);
}

void memory_init() {
    _G = (struct MemorySystemGlobals) {0};

    _G.default_allocator = malloc_allocator_create();

}

void memsys_shutdown() {
    malloc_allocator_destroy(_G.default_allocator);
}



