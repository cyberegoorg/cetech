#include <stddef.h>

#include <cetech/memory/allocator.h>
#include <cetech/handler/handlerid.h>
#include <cetech/containers/queue.inl>

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

struct handler_gen {
    struct cel_allocator * alloc;
    QUEUE_T(uint32_t) _freeIdx;
    ARRAY_T(uint32_t) _generation;
};

//==============================================================================
// Public interface
//==============================================================================

struct handler_gen *handlerid_create(struct cel_allocator *allocator) {
    struct handler_gen* hid = CEL_ALLOCATE(allocator, struct handler_gen, sizeof(struct handler_gen));

    hid->alloc = allocator;
    ARRAY_INIT(uint32_t, &hid->_generation, allocator);
    QUEUE_INIT(uint32_t, &hid->_freeIdx, allocator);

    handlerid_handler_create(hid);

    return hid;
}

void handlerid_destroy(struct handler_gen *hid) {
    ARRAY_DESTROY(uint32_t, &hid->_generation);
    QUEUE_DESTROY(uint32_t, &hid->_freeIdx);
    CEL_DEALLOCATE(hid->alloc, hid);
}

handler_t handlerid_handler_create(struct handler_gen *hid) {
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

void handlerid_handler_destroy(struct handler_gen *hid,
                               handler_t h) {
    uint32_t id = _idx(h);

    ARRAY_AT(&hid->_generation, id) += 1;
    QUEUE_PUSH_BACK(uint32_t, &hid->_freeIdx, id);
}

int handlerid_handler_alive(struct handler_gen *hid,
                            handler_t h) {
    return ARRAY_AT(&hid->_generation, _idx(h)) == _gen(h);
}