//==============================================================================
// Include
//==============================================================================

#include <cstdio>

#include "celib/allocator.h"
#include "celib/map.inl"
#include "celib/buffer.inl"

#include "cetech/kernel/api_system.h"
#include "cetech/kernel/log.h"
#include "cetech/kernel/path.h"
#include "cetech/kernel/process.h"
#include "cetech/kernel/vio.h"
#include "cetech/kernel/hashlib.h"
#include <cetech/kernel/config.h>
#include "cetech/kernel/memory.h"

#include "cetech/modules/machine/machine.h"
#include "cetech/kernel/resource.h"

#include "shader_blob.h"

using namespace celib;
using namespace buffer;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_process_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_yamlng_a0);


namespace shader_compiler {

    static int _shaderc(const char *input,
                        const char *output,
                        const char *include_path,
                        const char *type,
                        const char *platform,
                        const char *profile) {

        celib::Buffer buffer(ct_memory_a0.main_allocator());

        char *shaderc = ct_resource_a0.compiler_external_join(
                ct_memory_a0.main_allocator(),
                "shaderc");

        buffer << shaderc;
        CEL_FREE(ct_memory_a0.main_allocator(), shaderc);

        buffer::printf(buffer,
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

        int status = ct_process_a0.exec(buffer::c_str(buffer));

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
                        struct ct_yamlng_document *doc,
                        ct_vio *build_vio,
                        ct_compilator_api *compilator_api) {
        auto a = ct_memory_a0.main_allocator();

        const char *vs_input = doc->get_string(doc->inst,
                                                ct_yamlng_a0.calc_key("vs_input"),
                                                "");

        const char *fs_input = doc->get_string(doc->inst,
                                                ct_yamlng_a0.calc_key("fs_input"),
                                                "");

        const char *source_dir = ct_resource_a0.compiler_get_source_dir();
        const char *core_dir = ct_resource_a0.compiler_get_core_dir();

        char *include_dir = ct_path_a0.join(a, 2, core_dir, "bgfxshaders");

        shader_blob::blob_t resource = {};

        // TODO: temp cel_alloc?
        char output_path[1024] = {};
        char tmp_filename[1024] = {};

        auto kernel_platform = ct_config_a0.find("kernel.platform");
        char *tmp_dir = ct_resource_a0.compiler_get_tmp_dir(
                a,
                ct_config_a0.get_string(kernel_platform));

        //////// VS
        compilator_api->add_dependency(filename, vs_input);

        char *input_path = ct_path_a0.join(a, 2, source_dir, vs_input);

        _gen_tmp_name(output_path, tmp_dir,
                      CETECH_ARRAY_LEN(tmp_filename),vs_input);

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
        compilator_api->add_dependency(filename, fs_input);

        input_path = ct_path_a0.join(a, 2, source_dir, fs_input);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      fs_input);

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
        CETECH_GET_API(api, ct_path_a0);
        CETECH_GET_API(api, ct_vio_a0);
        CETECH_GET_API(api, ct_process_a0);
        CETECH_GET_API(api, ct_log_a0);
        CETECH_GET_API(api, ct_hash_a0);
        CETECH_GET_API(api, ct_config_a0);
        CETECH_GET_API(api, ct_yamlng_a0);

        ct_resource_a0.compiler_register_yaml(ct_hash_a0.id64_from_str("shader"), compiler);

        return 1;
    }
}
