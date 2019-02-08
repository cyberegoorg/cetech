#include <memory.h>
#include <stdlib.h>

#include <celib/memory/allocator.h>
#include <celib/api.h>

#include <celib/memory/memory.h>
#include <celib/macros.h>
#include <celib/containers/hash.h>
#include <celib/containers/buffer.h>
#include <celib/id.h>
#include "celib/memory/allocator.h"
#include "celib/log.h"

#define LOG_WHERE "memory"
#define CE_TRACE_MEMORY 0

#define _G ct_memory_global

struct memory_trace {
    void *ptr;
    const char *filename;
    int line;
};

static struct _G {
#if CE_TRACE_MEMORY
    struct memory_trace *memory_trace_pool;
    struct ce_hash_t memory_trace_map;
    ce_spinlock_t0 lock;
#endif
    int _;
} _G = {};

static void *_reallocate(const struct ce_alloc_t0 *a,
                         void *ptr,
                         uint32_t size,
                         uint32_t align,
                         const char *filename,
                         uint32_t line) {
    CE_UNUSED(a);
    CE_UNUSED(align);
    CE_UNUSED(filename);
    CE_UNUSED(line);

    return realloc(ptr, size);
}

#if CE_TRACE_MEMORY
static struct ce_alloc_t0 _allocator = {
        .inst = NULL,
        .reallocate = _reallocate
};

static void _trace_alloc(void *ptr,
                         const char *filename,
                         uint32_t line) {
    struct memory_trace trace = {
            .filename = filename,
            .line = line,
            .ptr = ptr,
    };

    uint32_t idx;
    ce_os_thread_a0->spin_lock(&_G.lock);

    idx = ce_array_size(_G.memory_trace_pool);
    ce_array_push(_G.memory_trace_pool, trace, &_allocator);
    ce_hash_add(&_G.memory_trace_map, (uint64_t) ptr, idx, &_allocator);

    ce_os_thread_a0->spin_unlock(&_G.lock);
}

static void _trace_free(void *ptr) {
    uint64_t idx = ce_hash_lookup(&_G.memory_trace_map,
                                  (uint64_t) ptr,
                                  UINT64_MAX);

    if (idx == UINT64_MAX) {
        return;
    }

    ce_os_thread_a0->spin_lock(&_G.lock);
    uint64_t last_idx = ce_array_size(_G.memory_trace_pool) - 1;

    _G.memory_trace_pool[idx] = _G.memory_trace_pool[last_idx];
    ce_array_pop_back(_G.memory_trace_pool);

    ce_hash_add(&_G.memory_trace_map, (uint64_t) _G.memory_trace_pool[idx].ptr,
                idx, &_allocator);

    ce_hash_remove(&_G.memory_trace_map, (uint64_t) ptr);
    ce_os_thread_a0->spin_unlock(&_G.lock);
}

#endif

static void *_reallocate_traced(const struct ce_alloc_t0 *a,
                                void *ptr,
                                uint32_t size,
                                uint32_t align,
                                const char *filename,
                                uint32_t line) {
    CE_UNUSED(a);
    CE_UNUSED(align);
    CE_UNUSED(filename);
    CE_UNUSED(line);

    void *new_ptr = NULL;

    new_ptr = _reallocate(a, ptr, size, align, filename, line);

#if CE_TRACE_MEMORY
    _trace_free(ptr);
    if (size) {
        _trace_alloc(new_ptr, filename, line);
    }
#endif

    return new_ptr;
}


static struct ce_alloc_t0 _traced_allocator = {
        .inst = NULL,
        .reallocate = _reallocate_traced
};

char *str_dup(const char *s,
              struct ce_alloc_t0 *allocator) {
    const uint32_t size = strlen(s) + 1;

    char *d = CE_ALLOC(allocator, char, size);
    CE_ASSERT("string", d != NULL);

    memset(d, '\0', sizeof(char) * size);
    strcpy(d, s);

    return d;
}

static struct ce_memory_a0 _api = {
        .system = &_traced_allocator,
        .str_dup = str_dup,
};

struct ce_memory_a0 *ce_memory_a0 = &_api;

void memory_register_api(struct ce_api_a0 *api) {
    api->register_api(CE_MEMORY_API, &_api, sizeof(_api));
}

void memory_init() {
}

void memsys_shutdown() {
#if CE_TRACE_MEMORY
    const uint32_t size = ce_array_size(_G.memory_trace_pool);

    if (size)
        ce_log_a0->error(LOG_WHERE, "leak count %d\n", size);

    for (int i = 0; i < size; ++i) {
        struct memory_trace entry = _G.memory_trace_pool[i];

        ce_log_a0->warning(LOG_WHERE, "leak from %s:%d",
                           entry.filename, entry.line);
    }
#endif

    _G = (struct _G) {};
}
