#include <memory.h>
#include <stdint.h>

#define LOG_WHERE "memory"


void *memory_copy(void *__restrict dest, const void *__restrict src, size_t n) {
    return memcpy(dest, src, n);
}

const void *pointer_align_forward(const void *p, uint32_t align) {
    uintptr_t pi = (uintptr_t) p;
    const uint32_t mod = pi % align;
    if (mod)
        pi += (align - mod);
    return (void *) pi;
}

const void *pointer_add(const void *p, uint32_t bytes) {
    return (const void *) ((const char *) p + bytes);
}


const void *pointer_sub(const void *p, uint32_t bytes) {
    return (const void *) ((const char *) p - bytes);
}
