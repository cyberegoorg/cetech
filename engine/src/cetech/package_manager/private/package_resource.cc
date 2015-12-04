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

#include "yaml-cpp/yaml.h"

namespace cetech {
    namespace resource_package {
        static const StringId64_t _type_hash = stringid64::from_cstring("package");;

        StringId64_t type_hash() {
            return _type_hash;
        }


        void compile(const char* filename,
                     CompilatorAPI& compilator) {
            CE_UNUSED(filename);

            YAML::Node document;
            if (!compilator.resource_to_yaml(document)) {
                return;
            }

            Header header = {document.size()};
            compilator.write_to_build(&header, sizeof(Header));

            /* Prepare arrays structs */
            Array < TypeHeader > typesheader(memory_globals::default_allocator()); // TODO: TEMP ALLOCATOR
            Array < StringId64_t > names(memory_globals::default_allocator());     // TODO: TEMP ALLOCATOR

            uint32_t names_offset = sizeof(Header) + (sizeof(TypeHeader) * header.count);

            for (auto itr = document.begin(); itr != document.end(); ++itr) {
                const YAML::Node& ar = itr->second;

                CE_ASSERT(ar.IsSequence());

                const char* type_name = itr->first.Scalar().c_str();
                TypeHeader type_header = {
                    .type = murmur_hash_64(type_name, strlen(type_name), 22),
                    .count = ar.size(),
                    .offset = names_offset
                };

                names_offset += sizeof(StringId64_t) * ar.size();

                array::push_back(typesheader, type_header);

                for (std::size_t i = 0; i < ar.size(); ++i) {
                    const YAML::Node& v = ar[i];

                    CE_ASSERT(v.IsScalar());

                    const char* name = v.Scalar().c_str();

                    array::push_back(names, murmur_hash_64(name, strlen(name), 22));
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
