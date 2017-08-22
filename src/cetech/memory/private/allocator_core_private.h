#ifndef CETECH_CORE_ALLOCATOR_PRIVATE_H
#define CETECH_CORE_ALLOCATOR_PRIVATE_H

struct cel_alloc;
struct ct_api_a0;

namespace core_allocator {
    void register_api(ct_api_a0 *api);

    cel_alloc *get();
}

#endif //CETECH_CORE_ALLOCATOR_PRIVATE_H
