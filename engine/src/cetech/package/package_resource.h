#pragma once

#include "celib/string/stringid_types.h"
#include "celib/memory/memory.h"
#include "cetech/os/os.h"
#include "celib/crypto/murmur_hash.inl.h"
#include "cetech/filesystem/file.h"

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

        void compiler(FSFile* in, FSFile* out);
        void* loader(FSFile* f, Allocator& a);
        void unloader(Allocator& a, void* data);
    }
}