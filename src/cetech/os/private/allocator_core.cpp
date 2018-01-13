#include <stdint.h>

#if defined(CETECH_LINUX)
#include <malloc.h>
#elif defined(CETECH_DARWIN)
#include <sys/malloc.h>
#endif

#include <cetech/os/memory.h>
#include <cetech/api/api_system.h>

#include "celib/allocator.h"

static void *_reallocate(const cel_alloc *a,
                         void *ptr,
                         uint32_t size,
                         uint32_t align) {
    CEL_UNUSED(a);
    CEL_UNUSED(align);

    void *new_ptr = NULL;

    if (size) {
        new_ptr = realloc(ptr, size);

    } else {
        // free(ptr);
    }

    return new_ptr;
}

static cel_alloc_fce alloc_fce = {
        .reallocate = _reallocate
};

static cel_alloc _allocator = {
        .inst = NULL,
        .call = &alloc_fce,
};

namespace core_allocator {
    cel_alloc *get() {
        return &_allocator;
    }

    ct_core_allocator_a0 core_allocator_api = {
            .get_allocator = core_allocator::get
    };

    void register_api(ct_api_a0 *api) {
        api->register_api("ct_core_allocator_a0", &core_allocator_api);
    }
}


