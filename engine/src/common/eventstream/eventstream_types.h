#pragma once

#include "runtime/runtime_types.h"
#include "common/memory/memory_types.h"
#include "common/container/container_types.h"


namespace cetech {
    typedef uint32_t event_it;

    struct EventStreamHeader {
        uint32_t type;
        uint32_t size;
    };

    struct EventStream {
        Array < char > stream;
        
        EventStream(Allocator & allocator);
    };
}