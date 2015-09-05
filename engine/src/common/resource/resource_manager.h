#pragma once

#include <cinttypes>

#include "common/string/stringid_types.h"
#include "common/memory/memory_types.h"
#include "runtime/runtime_types.h"

namespace cetech {
    namespace resource_manager_globals {
        void init();
        void shutdown();
    }

    namespace resource_manager {
        typedef void (* resource_compiler_clb_t)(File&, File&);
        typedef void* (* resource_loader_clb_t)(File&, Allocator&);
        typedef void (* resource_unloader_clb_t)(Allocator&, void*);

        void register_compiler(StringId64_t type, resource_compiler_clb_t clb);
        void register_loader(StringId64_t type, resource_loader_clb_t clb);
        void register_unloader(StringId64_t type, resource_unloader_clb_t clb);

        void compile(const char* filename);

        void load(StringId64_t type, StringId64_t name);
        void unload(StringId64_t type, StringId64_t name);

        bool can_get(StringId64_t type, StringId64_t name);
        const void* get(StringId64_t type, StringId64_t name);
    }
}