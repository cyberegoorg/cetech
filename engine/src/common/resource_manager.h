#pragma once

#include <cinttypes>

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

        void load(uint64_t type, uint64_t name);
        bool can_get(uint64_t type, uint64_t name);
        const void* get(uint64_t type, uint64_t name);

        void compile(const char* filename);

        void register_compiler_clb(uint64_t type, resource_compiler_clb_t clb);
        void register_loader_clb(uint64_t type, resource_loader_clb_t clb);
    }
}