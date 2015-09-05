#pragma once

#include "runtime/runtime_types.h"
#include "common/string/stringid_types.h"
#include "common/memory/memory_types.h"

namespace cetech {
    namespace resource_lua {
        struct Resource {
            uint32_t type;
        };

        void init();
        StringId64_t type_hash();

        void compiler(File& in, File& out);
        void* loader(File& f, Allocator& a);
        void unloader(Allocator& a, void* data);

        const char* get_source(const Resource* rs);
    }
}