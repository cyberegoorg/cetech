#ifndef CETECH__API_H
#define CETECH__API_H

#ifdef __cplusplus
extern "C" {
#endif

struct cel_alloc;
struct ct_api_a0;

void api_init(struct cel_alloc *allocator);

void api_shutdown();

struct ct_api_a0 *api_v0();

#ifdef __cplusplus
}
#endif

#endif //CETECH__API_H
