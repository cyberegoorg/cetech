#include <memory.h>
#include "../../errors/errors.h"

#define LOG_WHERE "memory"

void* memory_malloc(size_t size) {
    CE_ASSERT(LOG_WHERE, size > 0);

    void* mem = malloc(size);
    CE_ASSERT(LOG_WHERE, mem != NULL);

    return mem;
}

void  memory_free(void *ptr) {
    free(ptr);
}

void* memory_copy(void *dest, const void* src, size_t n) {
    return memcpy(dest, src, n);
}