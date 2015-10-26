#pragma once

#include "celib/string/stringid_types.h"

namespace cetech {
    namespace package_manager {
        void load(StringId64_t name);
        void unload(StringId64_t name);

        bool is_loaded(StringId64_t name);
        void flush(StringId64_t name);

        void load_boot_package();
        void unload_boot_package();
    }

    namespace package_manager_globals {
        void init();
        void shutdown();
    }
}