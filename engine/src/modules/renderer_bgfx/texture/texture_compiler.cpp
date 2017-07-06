#ifdef CETECH_CAN_COMPILE
//==============================================================================
// Include
//==============================================================================

#include <cetech/celib/allocator.h>
#include <cetech/celib/map.inl>

#include <cetech/kernel/hash.h>
#include <cetech/kernel/memory.h>
#include <cetech/modules/application.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/os.h>


#include <cetech/modules/resource.h>
#include <cstdio>
#include <cetech/kernel/yaml.h>
#include <cetech/celib/string_stream.h>
#include "texture_blob.h"

using namespace cetech;
using namespace string_stream;

CETECH_DECL_API(ct_memory_api_v0);
CETECH_DECL_API(ct_resource_api_v0);
CETECH_DECL_API(ct_app_api_v0);
CETECH_DECL_API(ct_path_v0);
CETECH_DECL_API(ct_vio_api_v0);
CETECH_DECL_API(ct_process_api_v0);
CETECH_DECL_API(ct_log_api_v0);
CETECH_DECL_API(ct_hash_api_v0);

namespace texture_compiler {

    static int _texturec(const char *input,
                         const char *output,
                         int gen_mipmaps,
                         int is_normalmap) {

        string_stream::Buffer buffer(ct_memory_api_v0.main_allocator());

        char *texturec = ct_resource_api_v0.compiler_external_join(
                ct_memory_api_v0.main_allocator(),
                "texturec");

        buffer << texturec;
        CETECH_FREE(ct_memory_api_v0.main_allocator(), texturec);

        printf(buffer, " -f %s -o %s", input, output);

        if (gen_mipmaps) {
            buffer << " --mips";
        }

        if (is_normalmap) {
            buffer << " --normalmap";
        }

        int status = ct_process_api_v0.exec(c_str(buffer));

        ct_log_api_v0.info("application", "STATUS %d", status);

        return status;
    }

    static int _gen_tmp_name(char *tmp_filename,
                             const char *tmp_dir,
                             size_t max_len,
                             const char *filename) {

        auto a = ct_memory_api_v0.main_allocator();

        char dir[1024] = {0};
        ct_path_v0.dir(dir, CETECH_ARRAY_LEN(dir), filename);

        char *tmp_dirname = ct_path_v0.join(a, 2, tmp_dir, dir);
        ct_path_v0.make_path(tmp_dirname);

        int ret = snprintf(tmp_filename, max_len, "%s/%s.ktx", tmp_dirname,
                           ct_path_v0.filename(filename));

        CETECH_FREE(a, tmp_dirname);

        return ret;
    }

    static int compiler(const char *filename,
                        struct ct_vio *source_vio,
                        struct ct_vio *build_vio,
                        struct ct_compilator_api *compilator_api) {

        auto a = ct_memory_api_v0.main_allocator();

        // TODO: temp ct_allocator?
        char input_str[1024] = {0};
        char output_path[1024] = {0};
        char tmp_filename[1024] = {0};

        char source_data[source_vio->size(source_vio->inst) + 1];
        memset(source_data, 0, source_vio->size(source_vio->inst) + 1);
        source_vio->read(source_vio->inst, source_data, sizeof(char),
                         source_vio->size(source_vio->inst));

        yaml_document_t h;
        yaml_node_t root = yaml_load_str(source_data, &h);

        yaml_node_t input = yaml_get_node(root, "input");
        yaml_node_t n_gen_mipmaps = yaml_get_node(root, "gen_mipmaps");
        yaml_node_t n_is_normalmap = yaml_get_node(root, "is_normalmap");

        int gen_mipmaps = yaml_is_valid(n_gen_mipmaps) ? yaml_as_bool(
                n_gen_mipmaps)
                                                       : 0;
        int is_normalmap = yaml_is_valid(n_is_normalmap) ? yaml_as_bool(
                n_is_normalmap) : 0;

        const char *source_dir = ct_resource_api_v0.compiler_get_source_dir();


        char *tmp_dir = ct_resource_api_v0.compiler_get_tmp_dir(a,
                                                             ct_app_api_v0.platform());

        yaml_as_string(input, input_str, CETECH_ARRAY_LEN(input_str));

        char *input_path = ct_path_v0.join(a, 2, source_dir, input_str);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      input_str);

        int result = _texturec(input_path, output_path, gen_mipmaps,
                               is_normalmap);

        if (result != 0) {
            return 0;
        }

        struct ct_vio *tmp_file = ct_vio_api_v0.from_file(output_path,
                                                          VIO_OPEN_READ);
        char *tmp_data =
                CETECH_ALLOCATE(ct_memory_api_v0.main_allocator(), char,
                                tmp_file->size(tmp_file->inst) + 1);
        tmp_file->read(tmp_file->inst, tmp_data, sizeof(char),
                       tmp_file->size(tmp_file->inst));

        texture_blob::blob_t resource = {
                .size = (uint32_t) tmp_file->size(tmp_file->inst)
        };

        build_vio->write(build_vio->inst, &resource, sizeof(resource), 1);
        build_vio->write(build_vio->inst, tmp_data, sizeof(char),
                         resource.size);

        tmp_file->close(tmp_file->inst);

        compilator_api->add_dependency(filename, input_str);

        CETECH_FREE(a, tmp_data);
        CETECH_FREE(a, input_path);
        CETECH_FREE(a, tmp_dir);

        return 1;
    }

    int init(ct_api_v0 *api) {
        CETECH_GET_API(api, ct_memory_api_v0);
        CETECH_GET_API(api, ct_resource_api_v0);
        CETECH_GET_API(api, ct_app_api_v0);
        CETECH_GET_API(api, ct_path_v0);
        CETECH_GET_API(api, ct_vio_api_v0);
        CETECH_GET_API(api, ct_process_api_v0);
        CETECH_GET_API(api, ct_log_api_v0);
        CETECH_GET_API(api, ct_hash_api_v0);

        ct_resource_api_v0.compiler_register(ct_hash_api_v0.id64_from_str("texture"),
                                          compiler);

        return 1;
    }

}

#endif