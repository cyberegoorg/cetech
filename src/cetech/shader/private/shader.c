//==============================================================================
// Include
//==============================================================================

#include <stdio.h>

#include "corelib/allocator.h"
#include "corelib/buffer.inl"

#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/log.h"

#include "corelib/hashlib.h"
#include "cetech/machine/machine.h"

#include "cetech/resource/resource.h"

#include <corelib/module.h>
#include <cetech/shader/shader.h>
#include <cetech/renderer/renderer.h>

#include <corelib/os.h>
#include <corelib/ydb.h>
#include <corelib/config.h>
#include <corelib/yng.h>
#include <cetech/kernel/kernel.h>
#include <cetech/builddb/builddb.h>

//==============================================================================
// GLobals
//==============================================================================

#define _G ShaderResourceGlobals
struct _G {
    struct ct_alloc *allocator;
} _G;


//==============================================================================
// Resource
//==============================================================================


static int _shaderc(const char *input,
                    const char *output,
                    const char *include_path,
                    const char *type,
                    const char *platform,
                    const char *profile) {
    struct ct_alloc *a = ct_memory_a0->system;

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

    int status = ct_os_a0->process->exec(buffer);

    ct_log_a0->debug("shaderc", "STATUS %d", status);

    return status;
}

static int _gen_tmp_name(char *tmp_filename,
                         const char *tmp_dir,
                         size_t max_len,
                         const char *filename) {

    struct ct_alloc *a = ct_memory_a0->system;

    char dir[1024] = {};
    ct_os_a0->path->dir(dir, filename);

    char *tmp_dirname = NULL;
    ct_os_a0->path->join(&tmp_dirname, a, 2, tmp_dir, dir);

    ct_os_a0->path->make_path(tmp_dirname);

    int ret = snprintf(tmp_filename, max_len, "%s/%s.shaderc", tmp_dirname,
                       ct_os_a0->path->filename(filename));
    ct_buffer_free(tmp_dirname, a);

    return ret;
}


#if CT_PLATFORM_LINUX
const char *platform = "linux";
const char *vs_profile = "120";
const char *fs_profile = "120";
#elif CT_PLATFORM_OSX
//const char *platform = "osx";
//const char *vs_profile = "metal";
//const char *fs_profile = "metal";
const char *vs_profile = "120";
const char *fs_profile = "120";
#else
const char* platform = "windows";
const char* vs_profile = "vs_4_0";
const char* fs_profile = "ps_4_0";
#endif

static void _compile(const char* filename, uint64_t obj) {
    const char *vs_input = ct_cdb_a0->read_str(obj, SHADER_VS_INPUT, "");
    const char *fs_input = ct_cdb_a0->read_str(obj, SHADER_FS_INPUT, "");

    ct_builddb_a0->add_dependency(filename, vs_input);
    ct_builddb_a0->add_dependency(filename, fs_input);

    struct ct_alloc *a = ct_memory_a0->system;

    const char *source_dir =  ct_cdb_a0->read_str(ct_config_a0->obj(), CONFIG_SRC, "");
    const char *core_dir = ct_cdb_a0->read_str(ct_config_a0->obj(), CONFIG_CORE, "");

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(obj);

    char *include_dir = NULL;
    ct_os_a0->path->join(&include_dir, a, 2, core_dir, "bgfxshaders");

    // TODO: temp ct_alloc?
    char output_path[1024] = {};
    char tmp_filename[1024] = {};


    const char *platform;
    platform = ct_cdb_a0->read_str(ct_config_a0->obj(), CONFIG_PLATFORM, "");

    char *tmp_dir = ct_resource_a0->compiler_get_tmp_dir(a, platform);

    //////// VS
//    compilator_api->add_dependency(filename, vs_input);

    char *input_path = NULL;
    ct_os_a0->path->join(&input_path, a, 2, source_dir, vs_input);

    _gen_tmp_name(output_path, tmp_dir,
                  CT_ARRAY_LEN(tmp_filename), vs_input);

    int result = _shaderc(input_path, output_path, include_dir, "vertex",
                          platform, vs_profile);

    ct_buffer_free(input_path, a);

    if (result != 0) {
        ct_buffer_free(include_dir, a);
        return;
    }


    struct ct_vio *tmp_file;

    do {
        tmp_file = ct_os_a0->vio->from_file(output_path, VIO_OPEN_READ);
    } while (tmp_file == NULL);


    char *vs_data = CT_ALLOC(ct_memory_a0->system, char,
                             tmp_file->size(tmp_file) + 1);

    uint64_t vs_data_size = tmp_file->size(tmp_file);
    tmp_file->read(tmp_file, vs_data, sizeof(char), vs_data_size);
    tmp_file->close(tmp_file);

    ct_cdb_a0->set_blob(w, SHADER_VS_DATA, vs_data, vs_data_size);

    ///////

    //////// FS
//    compilator_api->add_dependency(filename, fs_input);
    ct_buffer_clear(input_path);

    ct_os_a0->path->join(&input_path, a, 2, source_dir, fs_input);

    _gen_tmp_name(output_path, tmp_dir, CT_ARRAY_LEN(tmp_filename),
                  fs_input);

    result = _shaderc(input_path, output_path, include_dir, "fragment",
                      platform, fs_profile);

    ct_buffer_free(input_path, a);

    if (result != 0) {
        ct_buffer_free(include_dir, a);
        return;
    }

    tmp_file = ct_os_a0->vio->from_file(output_path, VIO_OPEN_READ);
    char *fs_data = CT_ALLOC(ct_memory_a0->system, char,
                             tmp_file->size(tmp_file) + 1);

    uint64_t fs_data_size = tmp_file->size(tmp_file);
    tmp_file->read(tmp_file, fs_data, sizeof(char), fs_data_size);
    tmp_file->close(tmp_file);

    ct_cdb_a0->set_blob(w, SHADER_FS_DATA, fs_data, fs_data_size);
    ct_cdb_a0->write_commit(w);

    CT_FREE(a, vs_data);
    CT_FREE(a, fs_data);
    ct_buffer_free(include_dir, a);
}

void shader_compiler(const char *filename,
                     char **output) {
    struct ct_alloc *a = ct_memory_a0->system;

    uint64_t key[] = {
            ct_yng_a0->key("vs_input")
    };

    const char *vs_input = ct_ydb_a0->get_str(filename, key, 1, "");

    key[0] = ct_yng_a0->key("fs_input");
    const char *fs_input = ct_ydb_a0->get_str(filename, key, 1, "");

    uint64_t obj = ct_cdb_a0->create_object(ct_cdb_a0->db(), SHADER_TYPE);

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(obj);
    if (vs_input) {
        ct_cdb_a0->set_str(w, SHADER_VS_INPUT, vs_input);
    }

    if (fs_input) {
        ct_cdb_a0->set_str(w, SHADER_FS_INPUT, fs_input);
    }

    ct_cdb_a0->write_commit(w);

    _compile(filename, obj);

    ct_cdb_a0->dump(obj, output, a);
    ct_cdb_a0->destroy_object(obj);
}


static void online(uint64_t name,
                   struct ct_vio *input,
                   uint64_t obj) {
    const uint64_t size = input->size(input);
    char *data = CT_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

    ct_cdb_a0->load(ct_cdb_a0->db(), data, obj, _G.allocator);

//    ct_cdb_a0->register_notify(obj, _on_obj_change, NULL);

    uint64_t fs_blob_size = 0;
    void *fs_blob;
    fs_blob = ct_cdb_a0->read_blob(obj, SHADER_FS_DATA, &fs_blob_size, 0);

    uint64_t vs_blob_size = 0;
    void *vs_blob;
    vs_blob = ct_cdb_a0->read_blob(obj, SHADER_VS_DATA, &vs_blob_size, 0);

    const ct_render_memory_t *vs_mem = ct_renderer_a0->make_ref(vs_blob,
                                                                vs_blob_size);
    const ct_render_memory_t *fs_mem = ct_renderer_a0->make_ref(fs_blob,
                                                                fs_blob_size);

    ct_render_shader_handle_t vs_shader = ct_renderer_a0->create_shader(vs_mem);
    ct_render_shader_handle_t fs_shader = ct_renderer_a0->create_shader(fs_mem);

    ct_render_program_handle_t program;
    program = ct_renderer_a0->create_program(vs_shader, fs_shader, true);

    ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(obj);
    ct_cdb_a0->set_uint64(writer, SHADER_PROP, program.idx);
    ct_cdb_a0->write_commit(writer);
}

static void offline(uint64_t name,
                    uint64_t obj) {
    CT_UNUSED(name);

    const uint64_t program = ct_cdb_a0->read_uint64(obj, SHADER_PROP, 0);
    ct_renderer_a0->destroy_program((ct_render_program_handle_t){.idx=(uint16_t) program});
}


static uint64_t cdb_type() {
    return SHADER_TYPE;
}

void shader_compiler(const char *filename,
                     char **output);

static struct ct_resource_i0 ct_resource_i0 = {
        .cdb_type = cdb_type,
        .online = online,
        .offline = offline,
        .compilator = shader_compiler,
};

//==============================================================================
// Interface
//==============================================================================
int shader_init(struct ct_api_a0 *api) {
    _G = (struct _G){.allocator = ct_memory_a0->system};

    ct_api_a0->register_api(RESOURCE_I_NAME, &ct_resource_i0);

    return 1;
}

void shader_shutdown() {
}

ct_render_program_handle_t shader_get(uint64_t shader) {
    const uint64_t idx = ct_cdb_a0->read_uint64(shader, SHADER_PROP, 0);
    return (ct_render_program_handle_t) {.idx=(uint16_t) idx};
}

static struct ct_shader_a0 shader_api = {
        .get = shader_get
};

struct ct_shader_a0 *ct_shader_a0 = &shader_api;

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_shader_a0", &shader_api);
}

CETECH_MODULE_DEF(
        shader,
        {
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_resource_a0);
            CT_INIT_API(api, ct_os_a0);
            CT_INIT_API(api, ct_log_a0);
            CT_INIT_API(api, ct_hashlib_a0);
            CT_INIT_API(api, ct_cdb_a0);
            CT_INIT_API(api, ct_renderer_a0);
        },
        {
            CT_UNUSED(reload);
            _init_api(api);
            shader_init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            shader_shutdown();
        }
)