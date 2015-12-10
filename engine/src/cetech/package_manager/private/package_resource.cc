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

#include "yaml/yaml.h"

namespace cetech {
    namespace resource_package {
        static const StringId64_t _type_hash = stringid64::from_cstring("package");;

        StringId64_t type_hash() {
            return _type_hash;
        }


        void compile(const char* filename,
                     CompilatorAPI& compilator) {
            CE_UNUSED(filename);

            yaml_parser_t parser;
            yaml_token_t token;

            const unsigned char* name;
            const unsigned char* type_name;

            int resource_count = 0;
            int type_count = 0;

            TypeHeader type_header;
            Header header;
            uint32_t names_offset;

            char tmp[compilator.resource_file_size() + 1];
            memset(tmp, 0, compilator.resource_file_size() + 1);


            if (!yaml_parser_initialize(&parser)) {
                log::error("package_resource", "yaml: Failed to initialize parser!");
            }

            compilator.read_resource_file(tmp);
            yaml_parser_set_input_string(&parser, (unsigned char*)tmp, compilator.resource_file_size());

            /* Prepare arrays structs */
            Array < TypeHeader > typesheader(memory_globals::default_allocator()); // TODO: TEMP ALLOCATOR
            Array < StringId64_t > names(memory_globals::default_allocator());     // TODO: TEMP ALLOCATOR

            // begin
            yaml_parser_scan(&parser, &token);
            if (token.type != YAML_STREAM_START_TOKEN) {
                log::error("package_resource", "Invalid yaml");
                goto clean_up;
            }

            yaml_parser_scan(&parser, &token);
            if (token.type != YAML_BLOCK_MAPPING_START_TOKEN) {
                log::error("package_resource", "Root node must be maping");

                goto clean_up;
            }

            // overall types
            do {
                // TYPE
                yaml_parser_scan(&parser, &token);

                if (token.type == YAML_BLOCK_END_TOKEN) {
                    break;
                }

                if (token.type != YAML_KEY_TOKEN) {
                    log::error("package_resource", "Need key");
                    goto clean_up;
                }

                yaml_parser_scan(&parser, &token);
                if (token.type != YAML_SCALAR_TOKEN) {
                    log::error("package_resource", "Need key");
                    goto clean_up;
                }

                type_name = token.data.scalar.value;

                // Value
                yaml_parser_scan(&parser, &token);
                if (token.type != YAML_VALUE_TOKEN) {
                    log::error("package_resource", "Need value");
                    goto clean_up;
                }

                yaml_parser_scan(&parser, &token);
                if (token.type != YAML_BLOCK_SEQUENCE_START_TOKEN) {
                    log::error("package_resource", "Need sequence");
                    goto clean_up;
                }

                // Resource name
                resource_count = 0;
                do {
                    yaml_parser_scan(&parser, &token);
                    if (token.type == YAML_BLOCK_END_TOKEN) {
                        break;
                    }

                    if (token.type != YAML_BLOCK_ENTRY_TOKEN) {
                        log::error("package_resource", "Need block entry");
                        goto clean_up;
                    }

                    yaml_parser_scan(&parser, &token);
                    if (token.type != YAML_SCALAR_TOKEN) {
                        log::error("package_resource", "Need scalar");
                        goto clean_up;
                    }

                    name = token.data.scalar.value;
                    array::push_back(names, murmur_hash_64(name, strlen((const char*)name), 22));

                    ++resource_count;
                } while (true);

                type_header = {
                    .type = murmur_hash_64(type_name, strlen((const char*)type_name), 22),
                    .count = (uint64_t)resource_count,
                    .offset = 0
                };

                array::push_back(typesheader, type_header);

                ++type_count;
            } while (true);

            header = {(uint64_t)type_count};

            // calc offset
            names_offset = sizeof(Header) + (sizeof(TypeHeader) * type_count);
            for (auto it = array::begin(typesheader); it != array::end(typesheader); ++it) {
                it->offset = names_offset;
                names_offset += sizeof(StringId64_t) * it->count;
            }

            /* Write header */
            compilator.write_to_build(&header, sizeof(Header));

            /* Write types and names */
            compilator.write_to_build(array::begin(typesheader),
                                      sizeof(TypeHeader) * array::size(typesheader));

            compilator.write_to_build(array::begin(names),
                                      sizeof(StringId64_t) * array::size(names));

clean_up:
            yaml_token_delete(&token);
            yaml_parser_delete(&parser);
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
