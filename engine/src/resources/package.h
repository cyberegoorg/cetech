#pragma once

#include "common/string/stringid_types.h"
#include "common/memory/memory.h"
#include "runtime/runtime.h"
#include "common/crypto/murmur_hash.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace cetech {
    namespace resource_package {
        struct Header {
            uint64_t count;
        };

        struct Item {
            StringId64_t type;
            StringId64_t name;
        };

        void init();
        StringId64_t type_hash();

        void compiler(File& in, File& out);
        void* loader(File& f, Allocator& a);
        void unloader(Allocator& a, void* data);
    }
}