#ifdef CETECH_CAN_COMPILE

//==============================================================================
// Include
//==============================================================================

#include <cstdio>

#include "celib/allocator.h"
#include "celib/map.inl"

#include "cetech/core/hashlib/hashlib.h"
#include <cetech/application/application.h>
#include "cetech/core/memory/memory.h"
#include "cetech/engine/machine/machine.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/log/log.h"
#include "cetech/core/yaml/yaml.h"


#include "cetech/engine/resource/resource.h"
#include "celib/string_stream.h"
#include "shader_blob.h"
#include "cetech/core/os/path.h"
#include "cetech/core/os/process.h"
#include "cetech/core/os/vio.h"

using namespace celib;
using namespace string_stream;

CETECH_DECL_API(ct_memory_a0)
CETECH_DECL_API(ct_resource_a0)
CETECH_DECL_API(ct_app_a0)
CETECH_DECL_API(ct_path_a0)
CETECH_DECL_API(ct_vio_a0)
CETECH_DECL_API(ct_process_a0)
CETECH_DECL_API(ct_log_a0)
CETECH_DECL_API(ct_hash_a0)


namespace shader_compiler {

    static int _shaderc(const char *input,
                        const char *output,
                        const char *include_path,
                        const char *type,
                        const char *platform,
                        const char *profile) {

        string_stream::Buffer buffer(ct_memory_a0.main_allocator());

        char *shaderc = ct_resource_a0.compiler_external_join(
                ct_memory_a0.main_allocator(),
                "shaderc");

        buffer << shaderc;
        CEL_FREE(ct_memory_a0.main_allocator(), shaderc);

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

        int status = ct_process_a0.exec(string_stream::c_str(buffer));

        ct_log_a0.debug("shaderc", "STATUS %d", status);

        return status;
    }

    static int _gen_tmp_name(char *tmp_filename,
                             const char *tmp_dir,
                             size_t max_len,
                             const char *filename) {

        auto a = ct_memory_a0.main_allocator();

        char dir[1024] = {};
        ct_path_a0.dir(dir, filename);

        char *tmp_dirname = ct_path_a0.join(a, 2, tmp_dir, dir);
        ct_path_a0.make_path(tmp_dirname);

        int ret = snprintf(tmp_filename, max_len, "%s/%s.shaderc", tmp_dirname,
                           ct_path_a0.filename(filename));

        CEL_FREE(a, tmp_dirname);

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
                        ct_vio *source_vio,
                        ct_vio *build_vio,
                        ct_compilator_api *compilator_api) {
        auto a = ct_memory_a0.main_allocator();

        char source_data[source_vio->size(source_vio->inst) + 1];
        memset(source_data, 0, source_vio->size(source_vio->inst) + 1);
        source_vio->read(source_vio->inst, source_data, sizeof(char),
                         source_vio->size(source_vio->inst));

        yaml_document_t h;
        yaml_node_t root = yaml_load_str(source_data, &h);

        yaml_node_t vs_input = yaml_get_node(root, "vs_input");
        yaml_node_t fs_input = yaml_get_node(root, "fs_input");

        const char *source_dir = ct_resource_a0.compiler_get_source_dir();
        const char *core_dir = ct_resource_a0.compiler_get_core_dir();

        char *include_dir = ct_path_a0.join(a, 2, core_dir, "bgfxshaders");

        shader_blob::blob_t resource = {};

        // TODO: temp cel_alloc?
        char input_str[1024] = {};
        char output_path[1024] = {};
        char tmp_filename[1024] = {};

        char *tmp_dir = ct_resource_a0.compiler_get_tmp_dir(a,
                                                            ct_app_a0.platform());

        //////// VS
        yaml_as_string(vs_input, input_str, CETECH_ARRAY_LEN(input_str));
        compilator_api->add_dependency(filename, input_str);

        char *input_path = ct_path_a0.join(a, 2, source_dir, input_str);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      input_str);

        int result = _shaderc(input_path, output_path, include_dir, "vertex",
                              platform, vs_profile);

        CEL_FREE(a, input_path);

        if (result != 0) {
            CEL_FREE(a, include_dir);
            return 0;
        }

        ct_vio *tmp_file = ct_vio_a0.from_file(output_path,
                                               VIO_OPEN_READ);
        char *vs_data =
                CEL_ALLOCATE(ct_memory_a0.main_allocator(), char,
                             tmp_file->size(tmp_file->inst) + 1);
        tmp_file->read(tmp_file->inst, vs_data, sizeof(char),
                       tmp_file->size(tmp_file->inst));
        resource.vs_size = tmp_file->size(tmp_file->inst);
        tmp_file->close(tmp_file->inst);
        ///////

        //////// FS
        yaml_as_string(fs_input, input_str, CETECH_ARRAY_LEN(input_str));
        compilator_api->add_dependency(filename, input_str);

        input_path = ct_path_a0.join(a, 2, source_dir, input_str);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      input_str);

        result = _shaderc(input_path, output_path, include_dir, "fragment",
                          platform, fs_profile);

        CEL_FREE(a, input_path);

        if (result != 0) {
            CEL_FREE(a, include_dir);
            return 0;
        }

        tmp_file = ct_vio_a0.from_file(output_path, VIO_OPEN_READ);
        char *fs_data =
                CEL_ALLOCATE(ct_memory_a0.main_allocator(), char,
                             tmp_file->size(tmp_file->inst) + 1);
        tmp_file->read(tmp_file->inst, fs_data, sizeof(char),
                       tmp_file->size(tmp_file->inst));

        resource.fs_size = tmp_file->size(tmp_file->inst);

        tmp_file->close(tmp_file->inst);

        build_vio->write(build_vio->inst, &resource, sizeof(resource), 1);
        build_vio->write(build_vio->inst, vs_data, sizeof(char),
                         resource.vs_size);
        build_vio->write(build_vio->inst, fs_data, sizeof(char),
                         resource.fs_size);

        CEL_FREE(a, vs_data);
        CEL_FREE(a, fs_data);
        CEL_FREE(a, include_dir);

        return 1;
    }

    int init(ct_api_a0 *api) {
        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_resource_a0);
        CETECH_GET_API(api, ct_app_a0);
        CETECH_GET_API(api, ct_path_a0);
        CETECH_GET_API(api, ct_vio_a0);
        CETECH_GET_API(api, ct_process_a0);
        CETECH_GET_API(api, ct_log_a0);
        CETECH_GET_API(api, ct_hash_a0);

        ct_resource_a0.compiler_register(ct_hash_a0.id64_from_str("shader"),
                                         compiler);

        return 1;
    }
}

#endif