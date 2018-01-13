#ifndef CELIB_HANDLERID_TYPES_INL
#define CELIB_HANDLERID_TYPES_INL

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

struct cel_alloc;

namespace celib {
    template<typename T>
    struct Handler {
        Handler();

        Handler(cel_alloc *allocator);

        void init(cel_alloc *allocator);

        void destroy();

        uint32_t* _generation;
        uint32_t* _freeIdx;
        cel_alloc* alloc;
    };
}

#endif //CELIB_HANDLERID_TYPES_INL
