#pragma once

#include "runtime/runtime_types.h"
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
        Spinlock lock;

        EventStream(Allocator & allocator, const uint32_t init_size = 64 * 1024);
    };
}