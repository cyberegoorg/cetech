#include "resources/package.h"

#include "common/container/array.inl.h"
#include "common/string/stringid_types.h"
#include "common/memory/memory.h"
#include "runtime/runtime.h"
#include "common/string/stringid.inl.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

namespace cetech {
    namespace resource_package {
        static const StringId64_t _type_hash = stringid64::from_cstring("package");;

        StringId64_t type_hash() {
            return _type_hash;
        }


        void compiler(File& in, File& out) {
            size_t sz_in = runtime::file::size(in);

            char tmp[4096] = {0};
            runtime::file::read(in, tmp, sz_in, 4096);

            rapidjson::Document document;
            document.Parse(tmp);

            if (document.HasParseError()) {
                log::error("resource_package.compiler", "Parse error: %s", GetParseError_En(
                               document.GetParseError()), document.GetErrorOffset());
                return;
            }

            Header header = {document.MemberCount()};
            runtime::file::write(out, &header, sizeof(Header), 1);

            /* Prepare arrays structs */
            Array < TypeHeader > typesheader(memory_globals::default_allocator()); // TODO: TEMP ALLOCATOR
            Array < StringId64_t > names(memory_globals::default_allocator());     // TODO: TEMP ALLOCATOR

            uint32_t names_offset = sizeof(Header) + (sizeof(TypeHeader) * header.count);

            for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd();
                 ++itr) {
                const rapidjson::Value& ar = itr->value;
                CE_ASSERT(ar.IsArray());

                TypeHeader type_header = {
                    .type = murmur_hash_64(itr->name.GetString(), strlen(itr->name.GetString()), 22),
                    .count = ar.Size(),
                    .offset = names_offset
                };

                names_offset += sizeof(StringId64_t) * ar.Size();

                array::push_back(typesheader, type_header);

                for (rapidjson::SizeType i = 0; i < ar.Size(); ++i) {
                    const rapidjson::Value& v = ar[i];
                    CE_ASSERT(v.IsString());

                    array::push_back(names, murmur_hash_64(v.GetString(), strlen(v.GetString()), 22));
                }
            }

            /* Write types and names */
            runtime::file::write(out, array::begin(typesheader), sizeof(TypeHeader), array::size(typesheader));
            runtime::file::write(out, array::begin(names), sizeof(StringId64_t), array::size(names));
        }

        void* loader (File& f, Allocator& a) {
            const uint64_t f_sz = runtime::file::size(f);

            void* mem = a.allocate(f_sz);
            runtime::file::read(f, mem, sizeof(char), f_sz);

            return mem;
        }

        void unloader(Allocator& a, void* data) {
            a.deallocate(data);
        }
    }
}