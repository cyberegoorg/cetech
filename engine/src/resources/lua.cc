#pragma once

#include "resources/lua.h"

#include "common/stringid_types.h"
#include "common/memory/memory.h"
#include "runtime/runtime.h"
#include "common/murmur_hash.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace cetech {
    namespace resource_lua {
        static StringId64_t _type_hash;

        void init() {
            _type_hash = murmur_hash_64("lua", 3, 22);
        }

        StringId64_t type_hash() {
            return _type_hash;
        }


        void compiler(File& in, File& out) {
            size_t sz_in = runtime::file::size(in);

            char tmp[4096] = {0};
            runtime::file::read(in, tmp, sz_in, 4096);

            Resource r;
            r.type = 1;

            runtime::file::write(out, &r, sizeof(Resource), 1);

            runtime::file::write(out, tmp, sz_in, 1);
        }

        void* loader (File& f, Allocator& a) {
            const uint64_t f_sz = runtime::file::size(f);

            void* mem = a.allocate(f_sz + 1);
            runtime::file::read(f, mem, sizeof(char), f_sz);
            ((char*)mem)[f_sz] = '\0';

            return mem;
        }

        void unloader(Allocator& a, void* data) {
            a.deallocate(data);
        }

        const char* get_source(const Resource* rs) {
            return (const char*)(rs + 1);
        }
    }
}