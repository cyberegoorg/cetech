#include <stdlib.h>

#if defined(CETECH_LINUX)
#include <malloc.h>
#elif defined(CETECH_DARWIN)
#endif

#include <corelib/memory.h>
#include <corelib/api_system.h>
#include <corelib/macros.h>
#include "corelib/allocator.h"

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

    void *new_ptr = NULL;

    if (size) {
        new_ptr = realloc(ptr, size);
    } else {
        free(ptr);
    }

    return new_ptr;
}

static struct ct_alloc_fce alloc_fce = {
        .reallocate = _reallocate
};

static struct ct_alloc _allocator = {
        .inst = NULL,
        .call = &alloc_fce,
};

static struct ct_core_allocator_a0 core_allocator_api = {
        .alloc = &_allocator,
};

struct ct_core_allocator_a0 *ct_core_allocator_a0 = &core_allocator_api;

void coreallocator_register_api(struct ct_api_a0 *api) {
    api->register_api("ct_core_allocator_a0", ct_core_allocator_a0);
}
