#ifndef CETECH_HANDLERID_H
#define CETECH_HANDLERID_H

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

struct allocator;

//==============================================================================
// Typedef and struct
//==============================================================================

enum {
    HANDLER_API_ID = 1515313
};

//==============================================================================
// Public interface
//==============================================================================

struct handler_api_v0 {
    struct handler32gen *
    (*handler32gen_create)(struct allocator *allocator);

    void (*handler32gen_destroy)(struct handler32gen *hid);

    uint32_t (*handler32_create)(struct handler32gen *hid);

    void (*handler32_destroy)(struct handler32gen *hid,
                              uint32_t h);

    int (*handler32_alive)(struct handler32gen *hid,
                           uint32_t h);
};

#endif //CETECH_HANDLERID_H
