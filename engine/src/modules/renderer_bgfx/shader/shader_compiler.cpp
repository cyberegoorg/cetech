#ifdef CETECH_CAN_COMPILE

//==============================================================================
// Include
//==============================================================================

#include <cstdio>

#include <cetech/celib/allocator.h>
#include <cetech/celib/map.inl>

#include <cetech/kernel/hash.h>
#include <cetech/modules/application.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/os.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/yaml.h>


#include <cetech/modules/resource.h>
#include <cetech/celib/string_stream.h>
#include "shader_blob.h"

using namespace cetech;
using namespace string_stream;

CETECH_DECL_API(memory_api_v0)
CETECH_DECL_API(resource_api_v0)
CETECH_DECL_API(app_api_v0)
CETECH_DECL_API(os_path_v0)
CETECH_DECL_API(os_vio_api_v0)
CETECH_DECL_API(os_process_api_v0)
CETECH_DECL_API(log_api_v0)
CETECH_DECL_API(hash_api_v0)


namespace shader_compiler {

    static int _shaderc(const char *input,
                        const char *output,
                        const char *include_path,
                        const char *type,
                        const char *platform,
                        const char *profile) {

        string_stream::Buffer buffer(memory_api_v0.main_allocator());

        char *shaderc = resource_api_v0.compiler_external_join(
                memory_api_v0.main_allocator(),
                "shaderc");

        buffer << shaderc;
        CETECH_FREE(memory_api_v0.main_allocator(), shaderc);

        string_stream::printf(buffer,
                              ""
                                      " -f %s"
                                      " -o %s"
                                      " -i %s"
                                      " --type %s"
                                      " --platform %s"
                                      " --profile %s"

                                      " 2>&1",  // TODO: move to exec

                              input, output, include_path, type, platform,
                              profile);

        int status = os_process_api_v0.exec(string_stream::c_str(buffer));

        log_api_v0.debug("shaderc", "STATUS %d", status);

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

        int ret = snprintf(tmp_filename, max_len, "%s/%s.shaderc", tmp_dirname,
                           os_path_v0.filename(filename));

        CETECH_FREE(a, tmp_dirname);

        return ret;
    }


#if defined(CETECH_LINUX)
    const char *platform = "linux";
    const char *vs_profile = "120";
    const char *fs_profile = "120";
#elif defined(CETECH_DARWIN)
    const char* platform = "osx";
    const char* vs_profile = "120";
    const char* fs_profile = "120";
#else
    const char* platform = "windows";
    const char* vs_profile = "vs_4_0";
    const char* fs_profile = "ps_4_0";
#endif

    static int compiler(const char *filename,
                        struct os_vio *source_vio,
                        struct os_vio *build_vio,
                        struct compilator_api *compilator_api) {
        auto a = memory_api_v0.main_allocator();

        char source_data[os_vio_api_v0.size(source_vio) + 1];
        memset(source_data, 0, os_vio_api_v0.size(source_vio) + 1);
        os_vio_api_v0.read(source_vio, source_data, sizeof(char),
                           os_vio_api_v0.size(source_vio));

        yaml_document_t h;
        yaml_node_t root = yaml_load_str(source_data, &h);

        yaml_node_t vs_input = yaml_get_node(root, "vs_input");
        yaml_node_t fs_input = yaml_get_node(root, "fs_input");

        const char *source_dir = resource_api_v0.compiler_get_source_dir();
        const char *core_dir = resource_api_v0.compiler_get_core_dir();

        char *include_dir = os_path_v0.join(a, 2, core_dir, "bgfxshaders");

        shader_blob::blob_t resource = {0};

        // TODO: temp allocator?
        char input_str[1024] = {0};
        char output_path[1024] = {0};
        char tmp_filename[1024] = {0};

        char *tmp_dir = resource_api_v0.compiler_get_tmp_dir(a,
                                                             app_api_v0.platform());

        //////// VS
        yaml_as_string(vs_input, input_str, CETECH_ARRAY_LEN(input_str));
        compilator_api->add_dependency(filename, input_str);

        char *input_path = os_path_v0.join(a, 2, source_dir, input_str);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      input_str);

        int result = _shaderc(input_path, output_path, include_dir, "vertex",
                              platform, vs_profile);

        CETECH_FREE(a, input_path);

        if (result != 0) {
            CETECH_FREE(a, include_dir);
            return 0;
        }

        struct os_vio *tmp_file = os_vio_api_v0.from_file(output_path,
                                                          VIO_OPEN_READ);
        char *vs_data =
                CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                                os_vio_api_v0.size(tmp_file) + 1);
        os_vio_api_v0.read(tmp_file, vs_data, sizeof(char),
                           os_vio_api_v0.size(tmp_file));
        resource.vs_size = os_vio_api_v0.size(tmp_file);
        os_vio_api_v0.close(tmp_file);
        ///////

        //////// FS
        yaml_as_string(fs_input, input_str, CETECH_ARRAY_LEN(input_str));
        compilator_api->add_dependency(filename, input_str);

        input_path = os_path_v0.join(a, 2, source_dir, input_str);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      input_str);

        result = _shaderc(input_path, output_path, include_dir, "fragment",
                          platform, fs_profile);

        CETECH_FREE(a, input_path);

        if (result != 0) {
            CETECH_FREE(a, include_dir);
            return 0;
        }

        tmp_file = os_vio_api_v0.from_file(output_path, VIO_OPEN_READ);
        char *fs_data =
                CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                                os_vio_api_v0.size(tmp_file) + 1);
        os_vio_api_v0.read(tmp_file, fs_data, sizeof(char),
                           os_vio_api_v0.size(tmp_file));
        resource.fs_size = os_vio_api_v0.size(tmp_file);
        os_vio_api_v0.close(tmp_file);

        os_vio_api_v0.write(build_vio, &resource, sizeof(resource), 1);

        os_vio_api_v0.write(build_vio, vs_data, sizeof(char), resource.vs_size);
        os_vio_api_v0.write(build_vio, fs_data, sizeof(char), resource.fs_size);

        CETECH_FREE(a, vs_data);
        CETECH_FREE(a, fs_data);
        CETECH_FREE(a, include_dir);

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

        resource_api_v0.compiler_register(hash_api_v0.id64_from_str("shader"),
                                          compiler);

        return 1;
    }
}

#endif