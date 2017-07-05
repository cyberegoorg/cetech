#ifndef CETECH_ALLOCATOR_H
#define CETECH_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus

#include <new>

#endif

//==============================================================================
// Defines
//==============================================================================

enum {
    CETECH_SIZE_NOT_TRACKED =0xffffffffu
};

#define CETECH_ALLOCATE(a, T, size) (T*)((a)->reallocate((a)->inst,         \
                                                          NULL,              \
                                                          size,  \
                                                          CETECH_ALIGNOF(T)))

#define CETECH_ALLOCATE_ALIGN(a, T, size, align) (T*)((a)->reallocate((a)->inst,         \
                                                          NULL,              \
                                                          size,  \
                                                          align))

#define CETECH_FREE(a, p) ((a)->reallocate((a)->inst,p,0,0))

#define CETECH_NEW(a, T, ...)        (new (CETECH_ALLOCATE_ALIGN(a, T, sizeof(T), CETECH_ALIGNOF(T))) T(__VA_ARGS__))
#define CETECH_DELETE(a, T, p)    do {if (p) {(p)->~T(); CETECH_FREE(a,p);}} while (0)

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

typedef void allocator_instance_v0;

struct allocator {
    allocator_instance_v0* inst;

    void* (*reallocate)(allocator_instance_v0* a, void *ptr, uint32_t size, uint32_t align);

    uint32_t (*total_allocated)(struct allocator *allocator);
    uint32_t (*allocated_size)(void *p);
};


#endif //CETECH_ALLOCATOR_H
