//                  **Abstract memory alocator**
//

#ifndef CECORE_ALLOCATOR_H
#define CECORE_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>


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

struct ce_alloc_inst;
struct ce_alloc;

struct ce_alloc {
    struct ce_alloc_inst *inst;
    void *(*reallocate)(const struct ce_alloc *a,
                        void *ptr,
                        uint32_t size,
                        uint32_t align,
                        const char *filename,
                        uint32_t line);

    uint32_t (*total_allocated)(const struct ce_alloc *allocator);
};


#endif //CECORE_ALLOCATOR_H
