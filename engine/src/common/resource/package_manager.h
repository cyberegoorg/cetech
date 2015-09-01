#pragma once

#include <cinttypes>

#include "common/stringid_types.h"
#include "common/memory/memory_types.h"
#include "runtime/runtime_types.h"

namespace cetech {
    namespace package_manager_globals {
        void init();
        void shutdown();
    }

    namespace package_manager {
        StringId64_t type_name();

        void load(StringId64_t name);
        void unload(StringId64_t name);

        bool is_loaded(StringId64_t name);
        void flush(StringId64_t name);
    }
}