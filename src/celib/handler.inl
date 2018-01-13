#ifndef CELIB_HANDLERID_INL
#define CELIB_HANDLERID_INL

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

#include "container_types.inl"
#include "handler_types.inl"
#include "allocator.h"
#include "array.h"

struct cel_alloc;

//==============================================================================
// Public interface
//==============================================================================

#define _GENBITCOUNT   8
#define _INDEXBITCOUNT 24
#define _MINFREEINDEXS 1024

#define _idx(h) ((h) >> _INDEXBITCOUNT)
#define _gen(h) ((h) & ((1 << _GENBITCOUNT) - 1));
#define _make_entity(idx, gen) (uint32_t)(((idx) << _INDEXBITCOUNT) | (gen))

namespace celib {
    namespace handler {
        template<typename T>
        T create(Handler<T> &handler);

        template<typename T>
        void destroy(Handler<T> &handler,
                     const T &h);

        template<typename T>
        bool alive(Handler<T> &handler,
                   const T &h);
    };
};

namespace celib {

    namespace handler {
        template<typename T>
        T create(Handler<T> &handler) {
            T idx;

            if (cel_array_size(handler._freeIdx) > _MINFREEINDEXS) {
                idx = handler._freeIdx[0];
                cel_array_pop_front(handler._freeIdx);
            } else {
                cel_array_push(handler._generation, 0, handler.alloc);

                idx = cel_array_size(handler._generation) - 1;
            }

            return _make_entity(idx, handler._generation[idx]);
        }

        template<typename T>
        void destroy(Handler<T> &handler,
                     const T &h) {
            T id = _idx(h);

            handler._generation[id] += 1;
            cel_array_push(handler._freeIdx, id, handler.alloc);
        }

        template<typename T>
        bool alive(Handler<T> &handler,
                   const T &h) {
            return handler._generation[_idx(h)] == _gen(h);
        }
    };

    template<typename T>
    Handler<T>::Handler() : _generation(nullptr), _freeIdx(nullptr),
                            alloc(nullptr) {
    }

    template<typename T>
    Handler<T>::Handler(cel_alloc *allocator) : _generation(nullptr), _freeIdx(nullptr),
                                                alloc(allocator) {
    }

    template<typename T>
    void Handler<T>::init(cel_alloc *allocator) {
        alloc = allocator;
        handler::create(*this);
    }

    template<typename T>
    void Handler<T>::destroy() {
        cel_array_free(_freeIdx, alloc);
        cel_array_free(_generation, alloc);
    }
};

#undef _GENBITCOUNT
#undef _INDEXBITCOUNT
#undef _MINFREEINDEXS
#undef _idx
#undef _gen
#undef _make_entity

#endif //CELIB_HANDLERID_INL
