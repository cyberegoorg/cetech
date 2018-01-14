#ifndef CELIB_ALLOCATOR_H
#define CELIB_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

#include "macros.h"

#ifdef __cplusplus
#include <new>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Defines
//==============================================================================

#define CEL_ALLOCATE(a, T, size) (T*)((a)->call->reallocate((a),             \
                                                          NULL,              \
                                                          size,              \
                                                          CEL_ALIGNOF(T)))

#define CEL_ALLOCATE_ALIGN(a, T, size, align) (T*)((a)->call->reallocate((a), \
                                                          NULL,               \
                                                          size,               \
                                                          align))

#define CEL_FREE(a, p) ((a)->call->reallocate((a),p,0,0))

#define CEL_NEW(a, T, ...) (new (CEL_ALLOCATE_ALIGN(a, T, sizeof(T), \
                                    CEL_ALIGNOF(T))) T(__VA_ARGS__))

#define CEL_DELETE(a, T, p) do { if (p) {(p)->~T(); CEL_FREE(a,p);}} while (0)


enum {
    CELIB_SIZE_NOT_TRACKED = 0xffffffffu
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

typedef void cel_alloc_inst;

struct cel_alloc;

struct cel_alloc_fce {
    void *(*reallocate)(const struct cel_alloc *a,
                        void *ptr,
                        uint32_t size,
                        uint32_t align);

    uint32_t (*total_allocated)(struct cel_alloc *allocator);
};

struct cel_alloc {
    cel_alloc_inst *inst;
    struct cel_alloc_fce* call;
};

#ifdef __cplusplus
}
#endif

#endif //CELIB_ALLOCATOR_H
