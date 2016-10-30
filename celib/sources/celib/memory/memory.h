//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CELIB_MEMORY_H
#define CELIB_MEMORY_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>
#include <memory.h>

#include "types.h"
#include "../types.h"


//==============================================================================
// Memory
//==============================================================================

static void *memory_copy(void *__restrict dest,
                         const void *__restrict src,
                         size_t n) {
    return memcpy(dest, src, n);
}

static void *memory_set(void *__restrict dest,
                        int c,
                        size_t n) {
    return memset(dest, c, n);
}

static const void *pointer_align_forward(const void *p,
                                         uint32_t align) {
    uintptr_t pi = (uintptr_t) p;
    const uint32_t mod = pi % align;
    if (mod)
        pi += (align - mod);
    return (void *) pi;
}

static const void *pointer_add(const void *p,
                               uint32_t bytes) {
    return (const void *) ((const char *) p + bytes);
}


static const void *pointer_sub(const void *p,
                               uint32_t bytes) {
    return (const void *) ((const char *) p - bytes);
}

#endif //CELIB_MEMORY_H
