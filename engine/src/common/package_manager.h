#pragma once

#include <cinttypes>

#include "common/memory/memory_types.h"
#include "runtime/runtime_types.h"

namespace cetech {
    namespace package_manager_globals {
        void init();
        void shutdown();
    }

    namespace package_manager {
        void load(uint64_t name);
    }
}