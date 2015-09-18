#pragma once

#include "common/memory/memory_types.h"
#include "common/container/container_types.h"

namespace cetech {
    typedef char event_t;
    typedef uint32_t event_it;

    struct EventStreamHeader {
        uint32_t type;
        uint32_t size;
        event_t data;
    };

    struct EventStream {
        Array < event_t > stream;

        EventStream(Allocator & allocator);
    };
}