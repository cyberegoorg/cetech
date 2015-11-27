#include "cetech/renderer/texture_resource.h"

#include "celib/container/array.inl.h"
#include "celib/string/stringid_types.h"
#include "celib/memory/memory.h"
#include "celib/macros.h"
#include "celib/string/stringid.inl.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "soil/SOIL.h"
#include "cetech/renderer/private/image_DXT.h"
#include <cetech/cvars/cvars.h>

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

            rapidjson::Document document;
            if (!compilator.resource_to_json(document)) {
                return;
            }

            const char* input = document["input"].GetString();
            compilator.add_dependency(input);

            char full_input_path[1024] = {0};
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
