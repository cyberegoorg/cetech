#ifndef CELIB_ALLOCATOR_H
#define CELIB_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

#include "macros.h"

#ifdef __cplusplus

#include <new>

#endif

//==============================================================================
// Defines
//==============================================================================

#define CEL_ALLOCATE(a, T, size) (T*)((a)->reallocate((a)->inst,             \
                                                          NULL,              \
                                                          size,              \
                                                          CEL_ALIGNOF(T)))

#define CEL_ALLOCATE_ALIGN(a, T, size, align) (T*)((a)->reallocate((a)->inst, \
                                                          NULL,               \
                                                          size,               \
                                                          align))

#define CEL_FREE(a, p) ((a)->reallocate((a)->inst,p,0,0))

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

typedef void allocator_instance;

struct ct_allocator {
    allocator_instance *inst;

    void *(*reallocate)(allocator_instance *a,
                        void *ptr,
                        uint32_t size,
                        uint32_t align);

    uint32_t (*total_allocated)(struct ct_allocator *allocator);

    uint32_t (*allocated_size)(void *p);
};


#endif //CELIB_ALLOCATOR_H
