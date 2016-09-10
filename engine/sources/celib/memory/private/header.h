//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CETECH_HEADER_H
#define CETECH_HEADER_H

#include <stdint.h>
#include "celib/memory/memory.h"

struct Header {
    uint32_t size;
};

static const uint32_t HEADER_PAD_VALUE = 0xffffffffu;

static void *data_pointer(struct Header *header,
                          uint32_t align) {
    const void *p = header + 1;
    return (void *) pointer_align_forward(p, align);
}

static struct Header *header(void *data) {
    uint32_t *p = (uint32_t *) data;

    while (p[-1] == HEADER_PAD_VALUE)
        --p;

    return (struct Header *) p - 1;
}

static void fill(struct Header *header,
                 void *data,
                 uint32_t size) {
    header->size = size;
    uint32_t *p = (uint32_t *) (header + 1);
    while (p < (uint32_t *) data)
        *p++ = HEADER_PAD_VALUE;
}

#endif //CETECH_HEADER_H
