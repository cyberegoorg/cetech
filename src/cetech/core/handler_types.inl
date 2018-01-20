#ifndef CECORE_HANDLERID_TYPES_INL
#define CECORE_HANDLERID_TYPES_INL

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

struct ct_alloc;

namespace celib {
    template<typename T>
    struct Handler {
        Handler();

        Handler(ct_alloc *allocator);

        void init(ct_alloc *allocator);

        void destroy();

        uint32_t *_generation;
        uint32_t *_freeIdx;
        ct_alloc *alloc;
    };
}

#endif //CECORE_HANDLERID_TYPES_INL
