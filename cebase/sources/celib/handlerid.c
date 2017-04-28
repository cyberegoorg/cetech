#include "celib/queue.h"

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
    uint32_t h;
} handler_t;

struct handlerid {
    QUEUE_T(uint32_t) _freeIdx;
    ARRAY_T(uint32_t) _generation;
};

//==============================================================================
// Public interface
//==============================================================================

handler_t handlerid_handler_create(struct handlerid *hid);

void handlerid_init(struct handlerid *hid,
                    struct cel_allocator *allocator) {
    ARRAY_INIT(uint32_t, &hid->_generation, allocator);
    QUEUE_INIT(uint32_t, &hid->_freeIdx, allocator);

    handlerid_handler_create(hid);
}

void handlerid_destroy(struct handlerid *hid) {
    ARRAY_DESTROY(uint32_t, &hid->_generation);
    QUEUE_DESTROY(uint32_t, &hid->_freeIdx);
}

handler_t handlerid_handler_create(struct handlerid *hid) {
    uint32_t idx;

    if (QUEUE_SIZE(uint32_t, &hid->_freeIdx) > _MINFREEINDEXS) {
        idx = QUEUE_AT(&hid->_freeIdx, 0);
        QUEUE_POP_FRONT(uint32_t, &hid->_freeIdx);
    } else {
        ARRAY_PUSH_BACK(uint32_t, &hid->_generation, 0);
        idx = (uint32_t) (ARRAY_SIZE(&hid->_generation) - 1);
    }

    return _make_entity(idx, ARRAY_AT(&hid->_generation, idx));
}

void handlerid_handler_destroy(struct handlerid *hid,
                               handler_t h) {
    uint32_t id = _idx(h);

    ARRAY_AT(&hid->_generation, id) += 1;
    QUEUE_PUSH_BACK(uint32_t, &hid->_freeIdx, id);
}

int handlerid_handler_alive(struct handlerid *hid,
                            handler_t h) {
    return ARRAY_AT(&hid->_generation, _idx(h)) == _gen(h);
}