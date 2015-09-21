#pragma once

#include "runtime/runtime_types.h"
#include "common/memory/memory_types.h"
#include "common/container/container_types.h"


namespace cetech {
    struct EventStream {
        Array < char > stream;

        EventStream(Allocator & allocator);
    };
}