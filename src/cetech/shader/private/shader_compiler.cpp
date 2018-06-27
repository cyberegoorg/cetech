//==============================================================================
// Include
//==============================================================================

#include <cstdio>

#include "corelib/allocator.h"
#include "corelib/buffer.inl"

#include "corelib/api_system.h"
#include "corelib/log.h"
#include <corelib/os.h>
#include "corelib/hashlib.h"
#include <corelib/config.h>
#include <corelib/ydb.h>
#include <corelib/cdb.h>
#include "corelib/memory.h"

#include "cetech/machine/machine.h"
#include "cetech/resource/resource.h"

#include "shader_blob.h"

static int _shaderc(const char *input,
                    const char *output,
                    const char *include_path,
                    const char *type,
                    const char *platform,
                    const char *profile) {
    ct_alloc *a = ct_memory_a0->main_allocator();

    char *buffer = NULL;

    char *shaderc = ct_resource_a0->compiler_external_join(a, "shaderc");

    ct_buffer_printf(&buffer, a, "%s", shaderc);

    ct_buffer_free(shaderc, a);

    ct_buffer_printf(&buffer, a,
                     ""
                     " -f %s"
                     " -o %s"
                     " -i %s"
                     " --type %s"
                     " --platform %s"
                     " --profile %s"

                     " 2>&1",  // TODO: move to exec
                     input, output, include_path, type, platform, profile);

    int status = ct_os_a0->process_a0->exec(buffer);

    ct_log_a0->debug("shaderc", "STATUS %d", status);

    return status;
}

static int _gen_tmp_name(char *tmp_filename,
                         const char *tmp_dir,
                         size_t max_len,
                         const char *filename) {

    struct ct_alloc *a = ct_memory_a0->main_allocator();

    char dir[1024] = {};
    ct_os_a0->path_a0->dir(dir, filename);

    char *tmp_dirname = NULL;
    ct_os_a0->path_a0->join(&tmp_dirname, a, 2, tmp_dir, dir);

    ct_os_a0->path_a0->make_path(tmp_dirname);

    int ret = snprintf(tmp_filename, max_len, "%s/%s.shaderc", tmp_dirname,
                       ct_os_a0->path_a0->filename(filename));
    ct_buffer_free(tmp_dirname, a);

    return ret;
}


#if defined(CETECH_LINUX)
const char *platform = "linux";
const char *vs_profile = "120";
const char *fs_profile = "120";
#elif defined(CETECH_DARWIN)
const char *platform = "osx";
//const char *vs_profile = "metal";
//const char *fs_profile = "metal";
const char *vs_profile = "120";
const char *fs_profile = "120";
#else
const char* platform = "windows";
const char* vs_profile = "vs_4_0";
const char* fs_profile = "ps_4_0";
#endif

static void compiler(const char *filename,
                     char **output,
                     struct ct_compilator_api *compilator_api) {
    struct ct_alloc *a = ct_memory_a0->main_allocator();

    uint64_t key[] = {
            ct_yng_a0->key("vs_input")
    };

    const char *vs_input = ct_ydb_a0->get_str(filename, key, 1, "");

    key[0] = ct_yng_a0->key("fs_input");
    const char *fs_input = ct_ydb_a0->get_str(filename, key, 1, "");

    const char *source_dir = ct_resource_a0->compiler_get_source_dir();
    const char *core_dir = ct_resource_a0->compiler_get_core_dir();

    char *include_dir = NULL;
    ct_os_a0->path_a0->join(&include_dir, a, 2, core_dir, "bgfxshaders");

    shader_blob::blob_t resource = {};

    // TODO: temp ct_alloc?
    char output_path[1024] = {};
    char tmp_filename[1024] = {};

    char *tmp_dir = ct_resource_a0->compiler_get_tmp_dir(
            a,
            ct_cdb_a0->read_str(ct_config_a0->config_object(),
                                CT_ID64_0("core.platform"), ""));

    //////// VS
    compilator_api->add_dependency(filename, vs_input);

    char *input_path = NULL;
    ct_os_a0->path_a0->join(&input_path, a, 2, source_dir, vs_input);

    _gen_tmp_name(output_path, tmp_dir,
                  CT_ARRAY_LEN(tmp_filename), vs_input);

    int result = _shaderc(input_path, output_path, include_dir, "vertex",
                          platform, vs_profile);

    ct_buffer_free(input_path, a);

    if (result != 0) {
        CT_FREE(a, include_dir);
        return;
    }


    ct_vio *tmp_file;

    do {
        tmp_file = ct_os_a0->vio_a0->from_file(output_path, VIO_OPEN_READ);
    } while (tmp_file == NULL);


    char *vs_data = CT_ALLOC(ct_memory_a0->main_allocator(), char,
                             tmp_file->size(tmp_file) + 1);

    tmp_file->read(tmp_file, vs_data, sizeof(char), tmp_file->size(tmp_file));
    resource.vs_size = tmp_file->size(tmp_file);
    tmp_file->close(tmp_file);
    ///////

    //////// FS
    compilator_api->add_dependency(filename, fs_input);
    ct_buffer_clear(input_path);

    ct_os_a0->path_a0->join(&input_path, a, 2, source_dir, fs_input);

    _gen_tmp_name(output_path, tmp_dir, CT_ARRAY_LEN(tmp_filename),
                  fs_input);

    result = _shaderc(input_path, output_path, include_dir, "fragment",
                      platform, fs_profile);

    ct_buffer_free(input_path, a);

    if (result != 0) {
        ct_buffer_free(include_dir, a);
        return;
    }

    tmp_file = ct_os_a0->vio_a0->from_file(output_path, VIO_OPEN_READ);
    char *fs_data = CT_ALLOC(ct_memory_a0->main_allocator(), char,
                             tmp_file->size(tmp_file) + 1);
    tmp_file->read(tmp_file, fs_data, sizeof(char), tmp_file->size(tmp_file));

    resource.fs_size = tmp_file->size(tmp_file);

    tmp_file->close(tmp_file);

    ct_array_push_n(*output, &resource, sizeof(resource), a);
    ct_array_push_n(*output, vs_data, sizeof(char) * resource.vs_size, a);
    ct_array_push_n(*output, fs_data, sizeof(char) * resource.fs_size, a);

    CT_FREE(a, vs_data);
    CT_FREE(a, fs_data);

    ct_buffer_free(include_dir, a);

}

int shadercompiler_init(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_os_a0);
    CETECH_GET_API(api, ct_log_a0);
    CETECH_GET_API(api, ct_hashlib_a0);
    CETECH_GET_API(api, ct_config_a0);
    CETECH_GET_API(api, ct_yng_a0);
    CETECH_GET_API(api, ct_ydb_a0);
    CETECH_GET_API(api, ct_cdb_a0);

    ct_resource_a0->compiler_register("shader", compiler, true);

    return 1;
}
