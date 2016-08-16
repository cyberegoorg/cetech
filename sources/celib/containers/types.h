#ifndef CETECH_CONTAINERS_TYPES_H
#define CETECH_CONTAINERS_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include "../types.h"

//==============================================================================
// Event stream
//==============================================================================

struct event_header {
    u32 type;
    u64 size;
};

struct eventstream {
    struct array_u8 stream;
};

#endif //CETECH_CONTAINERS_TYPES_H
