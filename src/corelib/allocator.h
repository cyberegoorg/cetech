//                  **Abstract memory alocator**
//

#ifndef CECORE_ALLOCATOR_H
#define CECORE_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>


#define CT_ALLOC(a, T, size)                        \
    (T*)((a)->call->reallocate((a),                 \
                               NULL,                \
                               size,                \
                               CT_ALIGNOF(T),       \
                               __FILE__,            \
                               __LINE__))

#define CT_ALLOCATE_ALIGN(a, T, size, align) \
    (T*)((a)->call->reallocate((a),   \
                               NULL,                \
                               size,                \
                               align,               \
                               __FILE__,            \
                               __LINE__))

#define CT_FREE(a, p) \
    ((a)->call->reallocate((a),p,0,0, __FILE__, __LINE__))

typedef void ct_alloc_inst;

struct ct_alloc;

struct ct_alloc_fce {
    void *(*reallocate)(const struct ct_alloc *a,
                        void *ptr,
                        uint32_t size,
                        uint32_t align,
                        const char *filename,
                        uint32_t line);

    uint32_t (*total_allocated)(const struct ct_alloc *allocator);
};

struct ct_alloc {
    ct_alloc_inst *inst;
    struct ct_alloc_fce *call;
};


#endif //CECORE_ALLOCATOR_H
