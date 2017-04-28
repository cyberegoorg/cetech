#ifndef CELIB_HANDLERID_H
#define CELIB_HANDLERID_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/queue.h"
#include "types.h"

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
                    struct cel_allocator *allocator);

void handlerid_destroy(struct handlerid *hid);

handler_t handlerid_handler_create(struct handlerid *hid);


void handlerid_handler_destroy(struct handlerid *hid,
                               handler_t h);

int handlerid_handler_alive(struct handlerid *hid,
                            handler_t h);


#endif //CELIB_HANDLERID_H
