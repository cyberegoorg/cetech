#ifndef CELIB_HANDLERID_H
#define CELIB_HANDLERID_H

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Typedef and struct
//==============================================================================

typedef struct handler {
    uint32_t h;
} handler_t;

//==============================================================================
// Public interface
//==============================================================================

struct handler_gen* handlerid_create(struct cel_allocator *allocator);

void handlerid_destroy(struct handler_gen *hid);

handler_t handlerid_handler_create(struct handler_gen *hid);


void handlerid_handler_destroy(struct handler_gen *hid,
                               handler_t h);

int handlerid_handler_alive(struct handler_gen *hid,
                            handler_t h);


#endif //CELIB_HANDLERID_H
