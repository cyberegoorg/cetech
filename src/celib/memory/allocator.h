//                  **Abstract memory alocator**
//

#ifndef CE_MEMORY_ALLOCATOR_H
#define CE_MEMORY_ALLOCATOR_H


#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"


#define CE_ALLOC(a, T, size)                        \
    (T*)((a)->reallocate((a),                       \
                               NULL,                \
                               size,                \
                               CE_ALIGNOF(T),       \
                               __FILE__,            \
                               __LINE__))

#define CE_ALLOCATE_ALIGN(a, T, size, align)        \
    (T*)((a)->reallocate((a),                       \
                               NULL,                \
                               size,                \
                               align,               \
                               __FILE__,            \
                               __LINE__))

#define CE_FREE(a, p) \
    ((a)->reallocate((a),p,0,0, __FILE__, __LINE__))

typedef struct ce_alloc_inst_t0 ce_alloc_inst_t0;

typedef struct ce_alloc_t0 {
    struct ce_alloc_inst_t0 *inst;

    void *(*reallocate)(const struct ce_alloc_t0 *a,
                        void *ptr,
                        uint32_t size,
                        uint32_t align,
                        const char *filename,
                        uint32_t line);
} ce_alloc_t0;

#ifdef __cplusplus
};
#endif

#endif //CE_MEMORY_ALLOCATOR_H
