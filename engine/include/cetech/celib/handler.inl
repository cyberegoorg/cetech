#ifndef CELIB_HANDLERID_INL
#define CELIB_HANDLERID_INL

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

#include <cetech/celib/container_types.inl>
#include <cetech/celib/handler_types.inl>
#include <cetech/celib/allocator.h>
#include <cetech/celib/queue.inl>

struct ct_allocator;

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

            if (queue::size(handler._freeIdx) > _MINFREEINDEXS) {
                idx = handler._freeIdx[0];
                queue::pop_front(handler._freeIdx);
            } else {
                array::push_back(handler._generation, (uint32_t) 0);

                idx = array::size(handler._generation) - 1;
            }

            return _make_entity(idx, handler._generation[idx]);
        }

        template<typename T>
        void destroy(Handler<T> &handler,
                     const T &h) {
            T id = _idx(h);

            handler._generation[id] += 1;
            queue::push_back(handler._freeIdx, id);
        }

        template<typename T>
        bool alive(Handler<T> &handler,
                   const T &h) {
            return handler._generation[_idx(h)] == _gen(h);
        }
    };

    template<typename T>
    Handler<T>::Handler() {
    }

    template<typename T>
    Handler<T>::Handler(ct_allocator *allocator) : _generation(allocator),
                                                   _freeIdx(allocator) {
    }

    template<typename T>
    void Handler<T>::init(ct_allocator *allocator) {
        _generation.init(allocator);
        _freeIdx.init(allocator);

        handler::create(*this);
    }

    template<typename T>
    void Handler<T>::destroy() {
        _generation.destroy();
        _freeIdx.destroy();
    }
};

#undef _GENBITCOUNT
#undef _INDEXBITCOUNT
#undef _MINFREEINDEXS
#undef _idx
#undef _gen
#undef _make_entity

#endif //CELIB_HANDLERID_INL
