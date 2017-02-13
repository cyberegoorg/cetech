#ifndef CELIB_HANDLERID_H
#define CELIB_HANDLERID_H

//==============================================================================
// Includes
//==============================================================================

#include "../containers/queue.h"
#include "../types.h"


//==============================================================================
// Private defines
//==============================================================================

#define _GENBITCOUNT   8
#define _INDEXBITCOUNT 22
#define _MINFREEINDEXS 1024
#define _idx(h) ((h).h >> _INDEXBITCOUNT)
#define _gen(h) ((h.h) & ((1 << _GENBITCOUNT) - 1));
#define _make_entity(idx, gen) (handler_t){.h = ((idx) << _INDEXBITCOUNT) | (gen)}


//==============================================================================
// Typedef and struct
//==============================================================================

typedef struct handler {
    u32 h;
} handler_t;

struct handlerid {
    QUEUE_T(u32) _freeIdx;
    ARRAY_T(u32) _generation;
};

//==============================================================================
// Public interface
//==============================================================================

static inline handler_t handlerid_handler_create(struct handlerid *hid);

static inline void handlerid_init(struct handlerid *hid,
                                  struct cel_allocator *allocator) {
    ARRAY_INIT(u32, &hid->_generation, allocator);
    QUEUE_INIT(u32, &hid->_freeIdx, allocator);

    handlerid_handler_create(hid);
}

static inline void handlerid_destroy(struct handlerid *hid) {
    ARRAY_DESTROY(u32, &hid->_generation);
    QUEUE_DESTROY(u32, &hid->_freeIdx);
}

static inline handler_t handlerid_handler_create(struct handlerid *hid) {
    u32 idx;

    if (QUEUE_SIZE(u32, &hid->_freeIdx) > _MINFREEINDEXS) {
        idx = QUEUE_AT(&hid->_freeIdx, 0);
        QUEUE_POP_FRONT(u32, &hid->_freeIdx);
    } else {
        ARRAY_PUSH_BACK(u32, &hid->_generation, 0);
        idx = (u32) (ARRAY_SIZE(&hid->_generation) - 1);
    }

    return _make_entity(idx, ARRAY_AT(&hid->_generation, idx));
}

static inline void handlerid_handler_destroy(struct handlerid *hid,
                                             handler_t h) {
    u32 id = _idx(h);

    ARRAY_AT(&hid->_generation, id) += 1;
    QUEUE_PUSH_BACK(u32, &hid->_freeIdx, id);
}

static inline int handlerid_handler_alive(struct handlerid *hid,
                                          handler_t h) {
    return ARRAY_AT(&hid->_generation, _idx(h)) == _gen(h);
}

#undef  _GENBITCOUNT
#undef  _INDEXBITCOUNT
#undef  _MINFREEINDEXS
#undef _idx
#undef _gen
#undef _make_entity

#endif //CELIB_HANDLERID_H
