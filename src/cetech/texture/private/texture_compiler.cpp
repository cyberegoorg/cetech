//==============================================================================
// Include
//==============================================================================

#include "corelib/allocator.h"

#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/log.h"
#include "cetech/machine/machine.h"


#include "cetech/resource/resource.h"
#include <cstdio>
#include <corelib/config.h>
#include <corelib/ydb.h>
#include <corelib/cdb.h>
#include <corelib/array.inl>
#include "corelib/buffer.inl"
#include "texture_blob.h"
#include <corelib/os.h>

static int _texturec(const char *input,
                     const char *output,
                     int gen_mipmaps,
                     int is_normalmap) {
    ct_alloc *alloc = ct_memory_a0->main_allocator();
    char *buffer = NULL;

    char *texturec = ct_resource_a0->compiler_external_join(alloc, "texturec");

    ct_buffer_printf(&buffer, alloc, "%s", texturec);
    ct_buffer_free(texturec, alloc);

    ct_buffer_printf(&buffer, alloc, " -f %s -o %s", input, output);

    if (gen_mipmaps) {
        ct_buffer_printf(&buffer, alloc, " %s", "--mips");
    }

    if (is_normalmap) {
        ct_buffer_printf(&buffer, alloc, " %s", "--normalmap");
    }

    ct_buffer_printf(&buffer, alloc, " %s", "2>&1");

    int status = ct_os_a0->process_a0->exec(buffer);

    ct_log_a0->info("application", "STATUS %d", status);

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

    int ret = snprintf(tmp_filename, max_len, "%s/%s.ktx", tmp_dirname,
                       ct_os_a0->path_a0->filename(filename));

    ct_buffer_free(tmp_dirname, a);

    return ret;
}

static void compiler(const char *filename,
                     char **output,
                     struct ct_compilator_api *compilator_api) {

    struct ct_alloc *a = ct_memory_a0->main_allocator();

    auto platform = ct_cdb_a0->read_str(ct_config_a0->config_object(),
                                        CT_ID64_0("kernel.platform"), "");

    char output_path[1024] = {};
    char tmp_filename[1024] = {};

    uint64_t key[] = {
            ct_yng_a0->key("input")
    };

    const char *input_str = ct_ydb_a0->get_str(filename, key, 1, "");

    key[0] = ct_yng_a0->key("gen_mipmaps");
    bool gen_mipmaps = ct_ydb_a0->get_bool(filename, key, 1, true);

    key[0] = ct_yng_a0->key("is_normalmap");
    bool is_normalmap = ct_ydb_a0->get_bool(filename, key, 1, false);

    const char *source_dir = ct_resource_a0->compiler_get_source_dir();

    char *tmp_dir = ct_resource_a0->compiler_get_tmp_dir(a, platform);
    char *input_path = NULL;
    ct_os_a0->path_a0->join(&input_path, a, 2, source_dir, input_str);

    _gen_tmp_name(output_path, tmp_dir, CT_ARRAY_LEN(tmp_filename),
                  input_str);

    int result = _texturec(input_path, output_path, gen_mipmaps,
                           is_normalmap);

    if (result != 0) {
        return;
    }

    ct_vio *tmp_file = NULL;

    tmp_file = ct_os_a0->vio_a0->from_file(output_path,
                                           VIO_OPEN_READ);

    char *tmp_data =
            CT_ALLOC(ct_memory_a0->main_allocator(), char,
                     tmp_file->size(tmp_file) + 1);
    tmp_file->read(tmp_file, tmp_data, sizeof(char),
                   tmp_file->size(tmp_file));

    texture_blob::blob_t resource = {
            .size = (uint32_t) tmp_file->size(tmp_file)
    };

    ct_array_push_n(*output, &resource, sizeof(texture_blob::blob_t), a);
    ct_array_push_n(*output, tmp_data, resource.size, a);

    tmp_file->close(tmp_file);

    compilator_api->add_dependency(filename, input_str);

    CT_FREE(a, tmp_data);
    ct_buffer_free(input_path, a);
    ct_buffer_free(tmp_dir, a);
}

int texturecompiler_init(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_os_a0);
    CETECH_GET_API(api, ct_log_a0);
    CETECH_GET_API(api, ct_hashlib_a0);
    CETECH_GET_API(api, ct_config_a0);
    CETECH_GET_API(api, ct_yng_a0);
    CETECH_GET_API(api, ct_ydb_a0);
    CETECH_GET_API(api, ct_cdb_a0);

    ct_resource_a0->compiler_register("texture", compiler, true);

    return 1;
}

