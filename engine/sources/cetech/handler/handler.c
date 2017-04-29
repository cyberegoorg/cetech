#include <stddef.h>

#include <cetech/allocator.h>
#include <cetech/handler.h>
#include <cetech/queue.inl>
#include <cetech/module.h>

//==============================================================================
// Private defines
//==============================================================================

#define _GENBITCOUNT   8
#define _INDEXBITCOUNT 24
#define _MINFREEINDEXS 1024
#define _idx(h) ((h).id >> _INDEXBITCOUNT)
#define _gen(h) ((h.id) & ((1 << _GENBITCOUNT) - 1));
#define _make_entity(idx, gen) (handler32_t){.id = ((idx) << _INDEXBITCOUNT) | (gen)}


//==============================================================================
// Typedef and struct
//==============================================================================

struct handler32gen {
    struct cel_allocator *alloc;
    QUEUE_T(uint32_t) _freeIdx;
    ARRAY_T(uint32_t) _generation;
};

//==============================================================================
// Public interface
//==============================================================================

struct handler32gen *handler32gen_create(struct cel_allocator *allocator) {
    struct handler32gen *hid = CEL_ALLOCATE(allocator, struct handler32gen, 1);

    hid->alloc = allocator;
    ARRAY_INIT(uint32_t, &hid->_generation, allocator);
    QUEUE_INIT(uint32_t, &hid->_freeIdx, allocator);

    handler32_t handler32_create(struct handler32gen *hid);
    handler32_create(hid);

    return hid;
}

void handler32gen_destroy(struct handler32gen *hid) {
    ARRAY_DESTROY(uint32_t, &hid->_generation);
    QUEUE_DESTROY(uint32_t, &hid->_freeIdx);
    CEL_DEALLOCATE(hid->alloc, hid);
}

handler32_t handler32_create(struct handler32gen *hid) {
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

void handler32_destroy(struct handler32gen *hid,
                       handler32_t h) {
    uint32_t id = _idx(h);

    ARRAY_AT(&hid->_generation, id) += 1;
    QUEUE_PUSH_BACK(uint32_t, &hid->_freeIdx, id);
}

int handler32_alive(struct handler32gen *hid,
                    handler32_t h) {
    return ARRAY_AT(&hid->_generation, _idx(h)) == _gen(h);
}

void *handler_get_module_api(int api) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID: {
            static struct module_api_v0 module = {0};
            return &module;
        }


        case HANDLER_API_ID: {
            static struct handler_api_v0 api = {
                    .handler32gen_create =  handler32gen_create,
                    .handler32gen_destroy =  handler32gen_destroy,
                    .handler32_create =  handler32_create,
                    .handler32_destroy =  handler32_destroy,
                    .handler32_alive =  handler32_alive
            };
            return &api;
        }


        default:
            return NULL;
    }
}