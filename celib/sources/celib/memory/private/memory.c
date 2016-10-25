#include <memory.h>
#include <stdint.h>

#include <celib/memory/memory.h>
#include "../../errors/errors.h"

#define LOG_WHERE "memory"


#define _G MemorySystemGlobals
struct G {
    struct allocator *default_allocator;
    struct allocator *default_scratch_allocator;
} MemorySystemGlobals = {0};

void memsys_init(int scratch_buffer_size) {
    _G = (struct G) {0};

    _G.default_allocator = malloc_allocator_create();

    _G.default_scratch_allocator = scratch_allocator_create(_G.default_allocator,
                                                            scratch_buffer_size);
}

void memsys_shutdown() {
    scratch_allocator_destroy(_G.default_scratch_allocator);
    malloc_allocator_destroy(_G.default_allocator);
}

struct allocator *memsys_main_allocator() {
    return _G.default_allocator;
}

struct allocator *memsys_main_scratch_allocator() {
    return _G.default_scratch_allocator;
}


void *memory_copy(void *__restrict dest,
                  const void *__restrict src,
                  size_t n) {
    return memcpy(dest, src, n);
}

void *memory_set(void *__restrict dest,
                 int c,
                 size_t n) {
    return memset(dest, c, n);
}

const void *pointer_align_forward(const void *p,
                                  uint32_t align) {
    uintptr_t pi = (uintptr_t) p;
    const uint32_t mod = pi % align;
    if (mod)
        pi += (align - mod);
    return (void *) pi;
}

const void *pointer_add(const void *p,
                        uint32_t bytes) {
    return (const void *) ((const char *) p + bytes);
}


const void *pointer_sub(const void *p,
                        uint32_t bytes) {
    return (const void *) ((const char *) p - bytes);
}

void *os_malloc(size_t size) {
    CE_ASSERT(LOG_WHERE, size > 0);

    void *mem = malloc(size);
    if (mem == NULL) {
        log_error(LOG_WHERE, "Malloc return NULL");
        return NULL;
    }

    return mem;
}

void os_free(void *ptr) {
    free(ptr);
}
