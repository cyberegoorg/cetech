#ifndef CETECH_CORE_ALLOCATOR_PRIVATE_H
#define CETECH_CORE_ALLOCATOR_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

struct cel_alloc;
struct ct_api_a0;

extern void coreallocator_register_api(struct ct_api_a0 *api);

extern struct cel_alloc *coreallocator_get();

#ifdef __cplusplus
}
#endif

#endif //CETECH_CORE_ALLOCATOR_PRIVATE_H
