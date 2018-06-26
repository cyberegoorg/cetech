#ifndef CETECH_HANDLER_H
#define CETECH_HANDLER_H

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "array.inl"


struct ct_handler_t {
    uint64_t *_generation;
    uint64_t *_freeIdx;
};

#define _GENBITCOUNT   8
#define _INDEXBITCOUNT 22
#define _MINFREEINDEXS 1024

#define _idx(h) ((h) >> _INDEXBITCOUNT)
#define _gen(h) ((h) & ((1 << (_GENBITCOUNT - 1))))
#define _make_entity(idx, gen) (uint64_t)(((idx) << _INDEXBITCOUNT) | (gen))

static inline uint64_t ct_handler_create(struct ct_handler_t *handler,
                                         const struct ct_alloc *allocator) {
    uint64_t idx;

    if (ct_array_size(handler->_freeIdx) > _MINFREEINDEXS) {
        idx = handler->_freeIdx[0];
        ct_array_pop_front(handler->_freeIdx);
    } else {
        ct_array_push(handler->_generation, 0, allocator);

        idx = ct_array_size(handler->_generation) - 1;
    }

    return _make_entity(idx, handler->_generation[idx]);
}

static inline void ct_handler_destroy(struct ct_handler_t *handler,
                                      uint64_t handlerid,
                                      const struct ct_alloc *allocator) {
    uint64_t id = _idx(handlerid);

    handler->_generation[id] += 1;
    ct_array_push(handler->_freeIdx, id, allocator);
}

static inline bool ct_handler_alive(struct ct_handler_t *handler,
                                    uint64_t handlerid) {
    return handler->_generation[_idx(handlerid)] == _gen(handlerid);
}

static inline void ct_handler_free(struct ct_handler_t *handler,
                                   const struct ct_alloc *allocator) {
    ct_array_free(handler->_freeIdx, allocator);
    ct_array_free(handler->_generation, allocator);
}


#endif //CETECH_HANDLER_H
