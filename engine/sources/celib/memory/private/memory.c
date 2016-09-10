#include <memory.h>
#include <stdint.h>

#include "../../errors/errors.h"

#define LOG_WHERE "memory"


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
