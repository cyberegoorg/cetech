#ifndef CELIB_HANDLERID_TYPES_INL
#define CELIB_HANDLERID_TYPES_INL

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

#include <cetech/celib/queue.inl>
#include <cetech/celib/array.inl>
#include <cetech/celib/allocator.h>
#include <cetech/celib/container_types.inl>

struct ct_allocator;

namespace celib {
    template<typename T>
    struct Handler {
        Handler();

        Handler(ct_allocator *allocator);

        void init(ct_allocator *allocator);

        void destroy();

        Array<uint32_t> _generation;
        Queue<uint32_t> _freeIdx;
    };
}

#endif //CELIB_HANDLERID_TYPES_INL
