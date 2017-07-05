#ifndef CETECH_CORE_ALLOCATOR_PRIVATE_H
#define CETECH_CORE_ALLOCATOR_PRIVATE_H

#include "allocator.inl"

namespace core_allocator {
    void register_api(api_v0 *api);
    allocator* get();
}

#endif //CETECH_CORE_ALLOCATOR_PRIVATE_H
