#include <memory.h>
#include <stdlib.h>

#include <corelib/api_system.h>
#include <corelib/os.h>
#include <corelib/memory.h>
#include <corelib/macros.h>
#include <corelib/hash.inl>
#include <corelib/buffer.inl>
#include "corelib/allocator.h"
#include "corelib/log.h"

#define LOG_WHERE "memory"
#define CT_TRACE_MEMORY 0

#define _G ct_memory_global

struct memory_trace {
    void *ptr;
    const char *filename;
    int line;
};

static struct _G {
#if CT_TRACE_MEMORY
    struct memory_trace *memory_trace_pool;
    struct ct_hash_t memory_trace_map;
    struct ct_spinlock lock;
#endif
    int _;
} _G = {0};

static void *_reallocate(const struct ct_alloc *a,
                         void *ptr,
                         uint32_t size,
                         uint32_t align,
                         const char *filename,
                         uint32_t line) {
    CT_UNUSED(a);
    CT_UNUSED(align);
    CT_UNUSED(filename);
    CT_UNUSED(line);

    return realloc(ptr, size);
}

#if CT_TRACE_MEMORY
static struct ct_alloc_fce alloc_fce = {
        .reallocate = _reallocate
};

static struct ct_alloc _allocator = {
        .inst = NULL,
        .call = &alloc_fce,
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
    ct_os_a0->thread->spin_lock(&_G.lock);

    idx = ct_array_size(_G.memory_trace_pool);
    ct_array_push(_G.memory_trace_pool, trace, &_allocator);
    ct_hash_add(&_G.memory_trace_map, (uint64_t) ptr, idx, &_allocator);

    ct_os_a0->thread->spin_unlock(&_G.lock);
}

static void _trace_free(void *ptr) {
    uint64_t idx = ct_hash_lookup(&_G.memory_trace_map,
                                  (uint64_t) ptr,
                                  UINT64_MAX);

    if (idx == UINT64_MAX) {
        return;
    }

    ct_os_a0->thread->spin_lock(&_G.lock);
    uint64_t last_idx = ct_array_size(_G.memory_trace_pool) - 1;

    _G.memory_trace_pool[idx] = _G.memory_trace_pool[last_idx];
    ct_array_pop_back(_G.memory_trace_pool);

    ct_hash_add(&_G.memory_trace_map, (uint64_t) _G.memory_trace_pool[idx].ptr,
                idx, &_allocator);

    ct_hash_remove(&_G.memory_trace_map, (uint64_t) ptr);
    ct_os_a0->thread->spin_unlock(&_G.lock);
}
#endif

static void *_reallocate_traced(const struct ct_alloc *a,
                                void *ptr,
                                uint32_t size,
                                uint32_t align,
                                const char *filename,
                                uint32_t line) {
    CT_UNUSED(a);
    CT_UNUSED(align);
    CT_UNUSED(filename);
    CT_UNUSED(line);

    void *new_ptr = NULL;

    new_ptr = _reallocate(a, ptr, size, align, filename, line);

#if CT_TRACE_MEMORY
    _trace_free(ptr);
    if (size) {
        _trace_alloc(new_ptr, filename, line);
    }
#endif

    return new_ptr;
}


static struct ct_alloc_fce traced_alloc_fce = {
        .reallocate = _reallocate_traced
};

static struct ct_alloc _traced_allocator = {
        .inst = NULL,
        .call = &traced_alloc_fce,
};

char *str_dup(const char *s,
              struct ct_alloc *allocator) {
    const uint32_t size = strlen(s) + 1;

    char *d = CT_ALLOC(allocator, char, size);
    CETECH_ASSERT("string", d != NULL);

    memset(d, '\0', sizeof(char) * size);
    strcpy(d, s);

    return d;
}

static struct ct_memory_a0 _api = {
        .system = &_traced_allocator,
        .str_dup = str_dup,
};

struct ct_memory_a0 *ct_memory_a0 = &_api;

void memory_register_api(struct ct_api_a0 *api) {
    api->register_api("ct_memory_a0", &_api);
}

void memory_init() {
}

void memsys_shutdown() {
#if CT_TRACE_MEMORY
    const uint32_t size = ct_array_size(_G.memory_trace_pool);

    if (size)
        ct_log_a0->error(LOG_WHERE, "leak count %d\n", size);

    for (int i = 0; i < size; ++i) {
        struct memory_trace entry = _G.memory_trace_pool[i];

        ct_log_a0->error(LOG_WHERE, "leak from %s:%d\n",
                         entry.filename, entry.line);
    }
#endif

    _G = (struct _G){0};
}
