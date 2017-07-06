#ifndef CETECH_CORE_ALLOCATOR_PRIVATE_H
#define CETECH_CORE_ALLOCATOR_PRIVATE_H

struct ct_allocator;
struct ct_api_a0;

namespace core_allocator {
    void register_api(ct_api_a0 *api);

    ct_allocator *get();
}

#endif //CETECH_CORE_ALLOCATOR_PRIVATE_H
