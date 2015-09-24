#pragma once

#include "common/string/stringid_types.h"
#include "common/memory/memory.h"
#include "runtime/runtime.h"
#include "common/crypto/murmur_hash.inl.h"
#include "filesystem/file.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace cetech {
    /*
     * Format.
     * [Header][TypeHeader1, TypeHeader2, ....][name11, name12, ...][name21, name22, ...]
     */
    namespace resource_package {
        struct Header {
            uint64_t count;
        };

        struct TypeHeader {
            StringId64_t type;
            uint64_t count;
            uint64_t offset;
        };

        StringId64_t type_hash();

        void compiler(File* in, File* out);
        void* loader(File* f, Allocator& a);
        void unloader(Allocator& a, void* data);
    }
}