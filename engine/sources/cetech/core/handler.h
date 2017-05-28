#ifndef CETECH_HANDLERID_H
#define CETECH_HANDLERID_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

struct allocator;

//==============================================================================
// Typedef and struct
//==============================================================================


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

#ifdef __cplusplus
}
#endif

#endif //CETECH_HANDLERID_H
