#ifndef CE_HANDLER_H
#define CE_HANDLER_H

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "array.inl"


struct ce_handler_t {
    char *_generation;
    uint64_t *_free_idx;
};

#define _GENBITCOUNT   8
#define _INDEXBITCOUNT 54
#define _MINFREEINDEXS 1024

#define handler_idx(h) ((h) >> _GENBITCOUNT)
#define handler_gen(h) ((h) & ((1 << (_GENBITCOUNT - 1))))

static inline uint64_t ce_handler_create(struct ce_handler_t *handler,
                                         const struct ce_alloc *allocator) {
    uint64_t idx;

    if (ce_array_size(handler->_free_idx) > _MINFREEINDEXS) {
        idx = handler->_free_idx[0];
        ce_array_pop_front(handler->_free_idx);
    } else {
        ce_array_push(handler->_generation, 0, allocator);

        idx = ce_array_size(handler->_generation) - 1;
    }

    uint64_t hid = ((idx) << _GENBITCOUNT) | (handler->_generation[idx]);

    return hid;
}

static inline void ce_handler_destroy(struct ce_handler_t *handler,
                                      uint64_t handlerid,
                                      const struct ce_alloc *allocator) {
    uint64_t id = handler_idx(handlerid);

    handler->_generation[id] += 1;
    ce_array_push(handler->_free_idx, id, allocator);
}

static inline bool ce_handler_alive(struct ce_handler_t *handler,
                                    uint64_t handlerid) {
    return handler->_generation[handler_idx(handlerid)] ==
           handler_gen(handlerid);
}

static inline void ce_handler_free(struct ce_handler_t *handler,
                                   const struct ce_alloc *allocator) {
    ce_array_free(handler->_free_idx, allocator);
    ce_array_free(handler->_generation, allocator);
}


#endif //CE_HANDLER_H
