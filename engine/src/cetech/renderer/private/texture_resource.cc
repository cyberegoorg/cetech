#include "cetech/renderer/texture_resource.h"

#include "celib/container/array.inl.h"
#include "celib/stringid_types.h"
#include "celib/memory/memory.h"
#include "celib/macros.h"
#include "celib/string/stringid.inl.h"

#include "yaml/yaml.h"

#include "soil/SOIL.h"
#include "cetech/renderer/private/image_DXT.h"
#include "cetech/cvars/cvars.h"

namespace cetech {
    namespace resource_texture {
        struct Header {
            uint32_t size;
        };

        static const StringId64_t _type_hash = stringid64::from_cstring("texture");

        StringId64_t type_hash() {
            return _type_hash;
        }


        void compile(const char* filename,
                     CompilatorAPI& compilator) {
            CE_UNUSED(filename);

            yaml_parser_t parser;
            yaml_token_t token;
            const char* input;
            char full_input_path[1024] = {0};

            char tmp[compilator.resource_file_size() + 1];
            memset(tmp, 0, compilator.resource_file_size() + 1);

            if (!yaml_parser_initialize(&parser)) {
                fputs("Failed to initialize parser!\n", stderr);
            }

            compilator.read_resource_file(tmp);
            yaml_parser_set_input_string(&parser, (unsigned char*)tmp, compilator.resource_file_size());

            yaml_parser_scan(&parser, &token);
            if (token.type != YAML_STREAM_START_TOKEN) {
                log::error("resource_compiler.texture", "Invalid yaml");
                goto clean_up;
            }

            yaml_parser_scan(&parser, &token);
            if (token.type != YAML_BLOCK_MAPPING_START_TOKEN) {
                log::error("resource_compiler.texture", "Root node must be maping");

                goto clean_up;
            }

            // Input
            yaml_parser_scan(&parser, &token);

            if (token.type != YAML_KEY_TOKEN) {
                log::error("resource_compiler.texture", "Need key");
                goto clean_up;
            }

            yaml_parser_scan(&parser, &token);
            if (token.type != YAML_SCALAR_TOKEN) {
                log::error("resource_compiler.texture", "Need key");
                goto clean_up;
            }

            // Value
            yaml_parser_scan(&parser, &token);
            if (token.type != YAML_VALUE_TOKEN) {
                log::error("package_resource", "Need value");
                goto clean_up;
            }

            yaml_parser_scan(&parser, &token);
            if (token.type != YAML_SCALAR_TOKEN) {
                log::error("package_resource", "Need sequence");
                goto clean_up;
            }

            input = (const char*)token.data.scalar.value;
            compilator.add_dependency(input);

            std::sprintf(full_input_path, "%s%s", cvars::rm_source_dir.value_str, input);

            unsigned char* data;
            unsigned char* dds_data;
            int w, h, ch, dds_size;

            data = dds_data = 0;
            w = h = ch = 0;

            data = SOIL_load_image(full_input_path, &w, &h, &ch, SOIL_LOAD_AUTO);
            if (!data) {
                log::error("resource_compiler.texture", "soil loda fail"); // TODO: error
                return;
            }

            dds_data = convert_image_to_DXT5(data, w, h, ch, &dds_size);


            Header header;
            header.size = dds_size;

            compilator.write_to_build(&header, sizeof(Header));
            compilator.write_to_build(dds_data, dds_size);

clean_up:
            yaml_token_delete(&token);
            yaml_parser_delete(&parser);
        }

        char* loader (FSFile& f,
                      Allocator& a) {
            const uint64_t f_sz = f.size();

            Header header;
            f.read(&header, sizeof(Header));

            const bgfx::Memory* mem = bgfx::alloc(header.size);
            f.read(mem->data, header.size);

            Resource* res = (Resource*) a.allocate(f_sz);

            res->mem = mem;

            return (char*)res;
        }

        void online(void* data) {
            Resource* res = (Resource*) data;
            res->handle = bgfx::createTexture(res->mem);
        }

        void offline(void* data) {
            Resource* res = (Resource*) data;
            bgfx::destroyTexture(res->handle);
        }

        void unloader(Allocator& a,
                      void* data) {
            a.deallocate(data);
        }
    }
}
