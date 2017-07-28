#include <memory.h>

#include <celib/allocator.h>
#include <cetech/core/os/errors.h>
#include <cetech/core/memory.h>
#include <cetech/core/api_system.h>
#include <cetech/core/log.h>

#include "memory_private.h"
#include "allocator_core_private.h"

CETECH_DECL_API(ct_log_a0);

#define LOG_WHERE "memory"
#define ALLOCATOR_WHERE "allocator"

struct MemorySystemGlobals {
    cel_alloc *default_allocator;
    cel_alloc *default_scratch_allocator;
} _G = {};


extern char *stacktrace(int);

extern void stacktrace_free(char *);

namespace memory {
    void allocator_trace_pointer(struct allocator_trace_entry *entries,
                                 uint32_t max_entries,
                                 void *p) {
        char *stacktrace_str = stacktrace(3);

        for (uint32_t i = 0; i < max_entries; ++i) {
            if (!entries[i].used) {
                entries[i].used = 1;
                entries[i].ptr = p;
                entries[i].stacktrace = stacktrace_str;
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

            stacktrace_free(entries[i].stacktrace);
            entries[i].stacktrace = NULL;
        }
    }

    void allocator_check_trace(struct allocator_trace_entry *entries,
                               uint32_t max_entries) {
        for (uint32_t i = 0; i < max_entries; ++i) {
            if (!entries[i].used) {
                continue;
            }

            ct_log_a0.error(ALLOCATOR_WHERE,
                            "memory_leak: %p\n  stacktrace:\n%s\n",
                            entries[i].ptr, entries[i].stacktrace);

            //allocator_free(cel_alloc, entries[i].ptr); // TODO: need this?

            stacktrace_free(entries[i].stacktrace);
        }
    }

    cel_alloc *memsys_main_allocator() {
        return _G.default_allocator;
    }

    cel_alloc *memsys_main_scratch_allocator() {
        return _G.default_scratch_allocator;
    }

    char *str_dup(const char *s,
                  cel_alloc *allocator) {
        char *d = (char *) CEL_ALLOCATE(allocator, char, strlen(s) + 1);
        CETECH_ASSERT("string", d != NULL);

        if (d == NULL) return NULL;

        strcpy(d, s);

        return d;
    }

    void register_api(ct_api_a0 *api) {
        CETECH_GET_API(api, ct_log_a0);

        static ct_memory_a0 _api = {};

        _api.main_allocator = memsys_main_allocator;
        _api.main_scratch_allocator = memsys_main_scratch_allocator;
        _api.str_dup = str_dup;

        api->register_api("ct_memory_a0", &_api);
    }

    void init(int scratch_buffer_size) {
        _G = {};

        _G.default_allocator = malloc_allocator_create();

        _G.default_scratch_allocator = scratch_allocator_create(
                _G.default_allocator,
                scratch_buffer_size);
    }

    void memsys_shutdown() {
        scratch_allocator_destroy(_G.default_scratch_allocator);
        malloc_allocator_destroy(_G.default_allocator);
    }
}


