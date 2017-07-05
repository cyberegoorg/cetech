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

CETECH_DECL_API(memory_api_v0);
CETECH_DECL_API(resource_api_v0);
CETECH_DECL_API(app_api_v0);
CETECH_DECL_API(os_path_v0);
CETECH_DECL_API(os_vio_api_v0);
CETECH_DECL_API(os_process_api_v0);
CETECH_DECL_API(log_api_v0);
CETECH_DECL_API(hash_api_v0);

namespace texture_compiler {

    static int _texturec(const char *input,
                         const char *output,
                         int gen_mipmaps,
                         int is_normalmap) {

        string_stream::Buffer buffer(memory_api_v0.main_allocator());

        char *texturec = resource_api_v0.compiler_external_join(
                memory_api_v0.main_allocator(),
                "texturec");

        buffer << texturec;
        CETECH_FREE(memory_api_v0.main_allocator(), texturec);

        printf(buffer, " -f %s -o %s", input, output);

        if (gen_mipmaps) {
            buffer << " --mips";
        }

        if (is_normalmap) {
            buffer << " --normalmap";
        }

        int status = os_process_api_v0.exec(c_str(buffer));

        log_api_v0.info("application", "STATUS %d", status);

        return status;
    }

    static int _gen_tmp_name(char *tmp_filename,
                             const char *tmp_dir,
                             size_t max_len,
                             const char *filename) {

        auto a = memory_api_v0.main_allocator();

        char dir[1024] = {0};
        os_path_v0.dir(dir, CETECH_ARRAY_LEN(dir), filename);

        char *tmp_dirname = os_path_v0.join(a, 2, tmp_dir, dir);
        os_path_v0.make_path(tmp_dirname);

        int ret = snprintf(tmp_filename, max_len, "%s/%s.ktx", tmp_dirname,
                           os_path_v0.filename(filename));

        CETECH_FREE(a, tmp_dirname);

        return ret;
    }

    static int compiler(const char *filename,
                        struct os_vio *source_vio,
                        struct os_vio *build_vio,
                        struct compilator_api *compilator_api) {

        auto a = memory_api_v0.main_allocator();

        // TODO: temp allocator?
        char input_str[1024] = {0};
        char output_path[1024] = {0};
        char tmp_filename[1024] = {0};

        char source_data[os_vio_api_v0.size(source_vio) + 1];
        memset(source_data, 0, os_vio_api_v0.size(source_vio) + 1);
        os_vio_api_v0.read(source_vio, source_data, sizeof(char),
                           os_vio_api_v0.size(source_vio));

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

        const char *source_dir = resource_api_v0.compiler_get_source_dir();


        char *tmp_dir = resource_api_v0.compiler_get_tmp_dir(a,
                                                             app_api_v0.platform());

        yaml_as_string(input, input_str, CETECH_ARRAY_LEN(input_str));

        char *input_path = os_path_v0.join(a, 2, source_dir, input_str);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      input_str);

        int result = _texturec(input_path, output_path, gen_mipmaps,
                               is_normalmap);

        if (result != 0) {
            return 0;
        }

        struct os_vio *tmp_file = os_vio_api_v0.from_file(output_path,
                                                          VIO_OPEN_READ);
        char *tmp_data =
                CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                                os_vio_api_v0.size(tmp_file) + 1);
        os_vio_api_v0.read(tmp_file, tmp_data, sizeof(char),
                           os_vio_api_v0.size(tmp_file));

        texture_blob::blob_t resource = {
                .size = (uint32_t) os_vio_api_v0.size(tmp_file)
        };

        os_vio_api_v0.write(build_vio, &resource, sizeof(resource), 1);
        os_vio_api_v0.write(build_vio, tmp_data, sizeof(char), resource.size);

        os_vio_api_v0.close(tmp_file);

        compilator_api->add_dependency(filename, input_str);

        CETECH_FREE(a, tmp_data);
        CETECH_FREE(a, input_path);
        CETECH_FREE(a, tmp_dir);

        return 1;
    }

    int init(api_v0 *api) {
        CETECH_GET_API(api, memory_api_v0);
        CETECH_GET_API(api, resource_api_v0);
        CETECH_GET_API(api, app_api_v0);
        CETECH_GET_API(api, os_path_v0);
        CETECH_GET_API(api, os_vio_api_v0);
        CETECH_GET_API(api, os_process_api_v0);
        CETECH_GET_API(api, log_api_v0);
        CETECH_GET_API(api, hash_api_v0);

        resource_api_v0.compiler_register(hash_api_v0.id64_from_str("texture"),
                                          compiler);

        return 1;
    }

}

#endif