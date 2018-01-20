#ifndef CECORE_ALLOCATOR_H
#define CECORE_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

#include "cetech/core/macros.h"

#ifdef __cplusplus
#include <new>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Defines
//==============================================================================

#define CT_ALLOC(a, T, size) (T*)((a)->call->reallocate((a),             \
                                                          NULL,              \
                                                          size,              \
                                                          CT_ALIGNOF(T)))

#define CT_ALLOCATE_ALIGN(a, T, size, align) (T*)((a)->call->reallocate((a), \
                                                          NULL,               \
                                                          size,               \
                                                          align))

#define CT_FREE(a, p) ((a)->call->reallocate((a),p,0,0))

#define CT_NEW(a, T, ...) (new (CT_ALLOCATE_ALIGN(a, T, sizeof(T), \
                                    CT_ALIGNOF(T))) T(__VA_ARGS__))

#define CT_DELETE(a, T, p) do { if (p) {(p)->~T(); CT_FREE(a,p);}} while (0)


enum {
    CECORE_SIZE_NOT_TRACKED = 0xffffffffu
};

//==============================================================================
// Defines
//==============================================================================

#define _4B  (  4 )
#define _8B  (  8 )
#define _16B ( 16 )
#define _32B ( 32 )
#define _64B ( 64 )

#define _1KiB  (  1024    )
#define _4KiB  ( _1KiB*4  )
#define _8KiB  ( _1KiB*8  )
#define _16KiB ( _1KiB*16 )
#define _32KiB ( _1KiB*32 )

//==============================================================================
// Allocator
//==============================================================================

typedef void ct_alloc_inst;

struct ct_alloc;

struct ct_alloc_fce {
    void *(*reallocate)(const struct ct_alloc *a,
                        void *ptr,
                        uint32_t size,
                        uint32_t align);

    uint32_t (*total_allocated)(struct ct_alloc *allocator);
};

struct ct_alloc {
    ct_alloc_inst *inst;
    struct ct_alloc_fce *call;
};

#ifdef __cplusplus
}
#endif

#endif //CECORE_ALLOCATOR_H
