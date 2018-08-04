//                  **Abstract memory alocator**
//

#ifndef CECORE_ALLOCATOR_H
#define CECORE_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>


#define CE_ALLOC(a, T, size)                        \
    (T*)((a)->call->reallocate((a),                 \
                               NULL,                \
                               size,                \
                               CE_ALIGNOF(T),       \
                               __FILE__,            \
                               __LINE__))

#define CE_ALLOCATE_ALIGN(a, T, size, align) \
    (T*)((a)->call->reallocate((a),   \
                               NULL,                \
                               size,                \
                               align,               \
                               __FILE__,            \
                               __LINE__))

#define CE_FREE(a, p) \
    ((a)->call->reallocate((a),p,0,0, __FILE__, __LINE__))

typedef void ce_alloc_inst;

struct ce_alloc;

struct ce_alloc_fce {
    void *(*reallocate)(const struct ce_alloc *a,
                        void *ptr,
                        uint32_t size,
                        uint32_t align,
                        const char *filename,
                        uint32_t line);

    uint32_t (*total_allocated)(const struct ce_alloc *allocator);
};

struct ce_alloc {
    ce_alloc_inst *inst;
    struct ce_alloc_fce *call;
};


#endif //CECORE_ALLOCATOR_H
