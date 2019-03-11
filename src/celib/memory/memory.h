#ifndef CE_MEMORY_H
#define CE_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#define CE_MEMORY_API \
    CE_ID64_0("ce_memory_a0", 0x5fc4626a7b1e5fdfULL)

typedef struct ce_alloc_t0 ce_alloc_t0;

//! Memory system API V0
struct ce_memory_a0 {
    ce_alloc_t0 *system;
    ce_alloc_t0 *virt_system;

    char *(*str_dup)(const char *s,
                     ce_alloc_t0 *allocator);
};

CE_MODULE(ce_memory_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_MEMORY_H
