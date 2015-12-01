#include "cetech/package_manager/package_resource.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "celib/container/array.inl.h"
#include "celib/stringid_types.h"
#include "celib/memory/memory.h"
#include "celib/macros.h"
#include "celib/string/stringid.inl.h"


namespace cetech {
    namespace resource_package {
        static const StringId64_t _type_hash = stringid64::from_cstring("package");;

        StringId64_t type_hash() {
            return _type_hash;
        }


        void compile(const char* filename,
                     CompilatorAPI& compilator) {
            CE_UNUSED(filename);

            rapidjson::Document document;
            if (!compilator.resource_to_json(document)) {
                return;
            }

            Header header = {document.MemberCount()};
            compilator.write_to_build(&header, sizeof(Header));

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
            compilator.write_to_build(array::begin(typesheader),
                                      sizeof(TypeHeader) * array::size(typesheader));

            compilator.write_to_build(array::begin(names),
                                      sizeof(StringId64_t) * array::size(names));
        }

        void online(void* data) {
            CE_UNUSED(data);
        }
        void offline(void* data) {
            CE_UNUSED(data);
        }

        char* loader (FSFile& f,
                      Allocator& a) {
            const uint64_t f_sz = f.size();

            char* mem = (char*) a.allocate(f_sz);
            f.read(mem, f_sz);

            return mem;
        }

        void unloader(Allocator& a,
                      void* data) {
            a.deallocate(data);
        }
    }
}
