#ifndef CETECH_HANDLERID_INL
#define CETECH_HANDLERID_INL

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

#include <cetech/core/container/queue2.inl>
#include <cetech/core/memory/allocator.h>
#include <cetech/core/container/container_types.inl>

struct allocator;

//==============================================================================
// Public interface
//==============================================================================

#define _GENBITCOUNT   8
#define _INDEXBITCOUNT 24
#define _MINFREEINDEXS 1024

#define _idx(h) ((h) >> _INDEXBITCOUNT)
#define _gen(h) ((h) & ((1 << _GENBITCOUNT) - 1));
#define _make_entity(idx, gen) (uint32_t)(((idx) << _INDEXBITCOUNT) | (gen))

namespace cetech {
    template<typename T>
    struct Handler {
        Handler();

        Handler(allocator *allocator);

        void init(allocator *allocator);

        void destroy();

        Array<uint32_t> _generation;
        Queue<uint32_t> _freeIdx;
    };

    template<typename T>
    Handler<T>::Handler() {
    }

    template<typename T>
    Handler<T>::Handler(allocator *allocator) : _generation(allocator),
                                             _freeIdx(allocator) {
    }

    template<typename T>
    void Handler<T>::init(allocator *allocator) {
        _generation.init(allocator);
        _freeIdx.init(allocator);
    }

    template<typename T>
    void Handler<T>::destroy() {
        _generation.destroy();
        _freeIdx.destroy();
    }


    namespace handler {
        template<typename T>
        T create(Handler<T> &handler) {
            T idx;

            if (queue::size(handler._freeIdx) > _MINFREEINDEXS) {
                idx = handler._freeIdx[0];
                queue::pop_front(handler._freeIdx);
            } else {
                array::push_back(handler._generation, (uint32_t)0);

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

};

#endif //CETECH_HANDLERID_INL
