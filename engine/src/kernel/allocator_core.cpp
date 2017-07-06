#include <stdint.h>
#include <malloc.h>

#include <cetech/celib/allocator.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/memory.h>

static void *_reallocate(allocator_instance *a,
                         void *ptr,
                         uint32_t size,
                         uint32_t align) {
    void *new_ptr = NULL;

    if (size)
        new_ptr = realloc(ptr, size);

    else
        free(ptr);

    return new_ptr;
}


static ct_allocator _allocator = {
        .inst = NULL,
        .reallocate= _reallocate
};


namespace core_allocator {
    ct_allocator *get() {
        return &_allocator;
    }

    ct_core_allocator_a0 core_allocator_api = {
        .get_allocator = core_allocator::get
    };

    void register_api(ct_api_a0 *api) {
        api->register_api("ct_core_allocator_a0", &core_allocator_api);
    }
}


