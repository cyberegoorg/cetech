//==============================================================================
// Include
//==============================================================================

#include <stdio.h>

#include "celib/allocator.h"
#include "celib/buffer.inl"

#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/log.h"

#include "celib/hashlib.h"
#include "cetech/machine/machine.h"

#include "cetech/resource/resource.h"

#include <celib/module.h>
#include <cetech/gfx/shader.h>
#include <cetech/gfx/renderer.h>

#include <celib/os.h>
#include <celib/ydb.h>
#include <celib/config.h>
#include <celib/yng.h>
#include <cetech/kernel/kernel.h>
#include <cetech/resource/builddb.h>

//==============================================================================
// GLobals
//==============================================================================

#define _G ShaderResourceGlobals
struct _G {
    struct ce_alloc *allocator;
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
    struct ce_alloc *a = ce_memory_a0->system;

    char *buffer = NULL;

    char *shaderc = ct_resource_a0->compiler_external_join(a, "shaderc");

    ce_buffer_printf(&buffer, a, "%s", shaderc);

    ce_buffer_free(shaderc, a);

    ce_buffer_printf(&buffer, a,
                     ""
                     " -f %s"
                     " -o %s"
                     " -i %s"
                     " --type %s"
                     " --platform %s"
                     " --profile %s"

                     " 2>&1",  // TODO: move to exec
                     input, output, include_path, type, platform, profile);

    int status = ce_os_a0->process->exec(buffer);

    ce_log_a0->debug("shaderc", "STATUS %d", status);

    return status;
}

static int _gen_tmp_name(char *tmp_filename,
                         const char *tmp_dir,
                         size_t max_len,
                         const char *filename) {

    struct ce_alloc *a = ce_memory_a0->system;

    char dir[1024] = {};
    ce_os_a0->path->dir(dir, filename);

    char *tmp_dirname = NULL;
    ce_os_a0->path->join(&tmp_dirname, a, 2, tmp_dir, dir);

    ce_os_a0->path->make_path(tmp_dirname);

    int ret = snprintf(tmp_filename, max_len, "%s/%s.shaderc", tmp_dirname,
                       ce_os_a0->path->filename(filename));
    ce_buffer_free(tmp_dirname, a);

    return ret;
}


#if CE_PLATFORM_LINUX
const char *platform = "linux";
const char *vs_profile = "120";
const char *fs_profile = "120";
#elif CE_PLATFORM_OSX
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
    const char *vs_input = ce_cdb_a0->read_str(obj, SHADER_VS_INPUT, "");
    const char *fs_input = ce_cdb_a0->read_str(obj, SHADER_FS_INPUT, "");

    ct_builddb_a0->add_dependency(filename, vs_input);
    ct_builddb_a0->add_dependency(filename, fs_input);

    struct ce_alloc *a = ce_memory_a0->system;

    const char *source_dir =  ce_cdb_a0->read_str(ce_config_a0->obj(), CONFIG_SRC, "");
    const char *core_dir = ce_cdb_a0->read_str(ce_config_a0->obj(), CONFIG_CORE, "");

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);

    char *include_dir = NULL;
    ce_os_a0->path->join(&include_dir, a, 2, core_dir, "bgfxshaders");

    // TODO: temp ce_alloc?
    char output_path[1024] = {};
    char tmp_filename[1024] = {};


    const char *platform;
    platform = ce_cdb_a0->read_str(ce_config_a0->obj(), CONFIG_PLATFORM, "");

    char *tmp_dir = ct_resource_a0->compiler_get_tmp_dir(a, platform);

    //////// VS
//    compilator_api->add_dependency(filename, vs_input);

    char *input_path = NULL;
    ce_os_a0->path->join(&input_path, a, 2, source_dir, vs_input);

    _gen_tmp_name(output_path, tmp_dir,
                  CE_ARRAY_LEN(tmp_filename), vs_input);

    int result = _shaderc(input_path, output_path, include_dir, "vertex",
                          platform, vs_profile);

    ce_buffer_free(input_path, a);

    if (result != 0) {
        ce_buffer_free(include_dir, a);
        return;
    }


    struct ce_vio *tmp_file;

    do {
        tmp_file = ce_os_a0->vio->from_file(output_path, VIO_OPEN_READ);
    } while (tmp_file == NULL);


    char *vs_data = CE_ALLOC(ce_memory_a0->system, char,
                             tmp_file->size(tmp_file) + 1);

    uint64_t vs_data_size = tmp_file->size(tmp_file);
    tmp_file->read(tmp_file, vs_data, sizeof(char), vs_data_size);
    tmp_file->close(tmp_file);

    ce_cdb_a0->set_blob(w, SHADER_VS_DATA, vs_data, vs_data_size);

    ///////

    //////// FS
//    compilator_api->add_dependency(filename, fs_input);
    ce_buffer_clear(input_path);

    ce_os_a0->path->join(&input_path, a, 2, source_dir, fs_input);

    _gen_tmp_name(output_path, tmp_dir, CE_ARRAY_LEN(tmp_filename),
                  fs_input);

    result = _shaderc(input_path, output_path, include_dir, "fragment",
                      platform, fs_profile);

    ce_buffer_free(input_path, a);

    if (result != 0) {
        ce_buffer_free(include_dir, a);
        return;
    }

    tmp_file = ce_os_a0->vio->from_file(output_path, VIO_OPEN_READ);
    char *fs_data = CE_ALLOC(ce_memory_a0->system, char,
                             tmp_file->size(tmp_file) + 1);

    uint64_t fs_data_size = tmp_file->size(tmp_file);
    tmp_file->read(tmp_file, fs_data, sizeof(char), fs_data_size);
    tmp_file->close(tmp_file);

    ce_cdb_a0->set_blob(w, SHADER_FS_DATA, fs_data, fs_data_size);
    ce_cdb_a0->write_commit(w);

    CE_FREE(a, vs_data);
    CE_FREE(a, fs_data);
    ce_buffer_free(include_dir, a);
}

void shader_compiler(const char *filename,
                     char **output) {
    struct ce_alloc *a = ce_memory_a0->system;

    uint64_t key[] = {
            ce_yng_a0->key("vs_input")
    };

    const char *vs_input = ce_ydb_a0->get_str(filename, key, 1, "");

    key[0] = ce_yng_a0->key("fs_input");
    const char *fs_input = ce_ydb_a0->get_str(filename, key, 1, "");

    uint64_t obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), SHADER_TYPE);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
    if (vs_input) {
        ce_cdb_a0->set_str(w, SHADER_VS_INPUT, vs_input);
    }

    if (fs_input) {
        ce_cdb_a0->set_str(w, SHADER_FS_INPUT, fs_input);
    }

    ce_cdb_a0->write_commit(w);

    _compile(filename, obj);

    ce_cdb_a0->dump(obj, output, a);
    ce_cdb_a0->destroy_object(obj);
}


static void online(uint64_t name,
                   uint64_t obj) {
//    ce_cdb_a0->register_notify(obj, _on_obj_change, NULL);

    uint64_t fs_blob_size = 0;
    void *fs_blob;
    fs_blob = ce_cdb_a0->read_blob(obj, SHADER_FS_DATA, &fs_blob_size, 0);

    uint64_t vs_blob_size = 0;
    void *vs_blob;
    vs_blob = ce_cdb_a0->read_blob(obj, SHADER_VS_DATA, &vs_blob_size, 0);

    const ct_render_memory_t *vs_mem = ct_renderer_a0->make_ref(vs_blob,
                                                                vs_blob_size);
    const ct_render_memory_t *fs_mem = ct_renderer_a0->make_ref(fs_blob,
                                                                fs_blob_size);

    ct_render_shader_handle_t vs_shader = ct_renderer_a0->create_shader(vs_mem);
    ct_render_shader_handle_t fs_shader = ct_renderer_a0->create_shader(fs_mem);

    ct_render_program_handle_t program;
    program = ct_renderer_a0->create_program(vs_shader, fs_shader, true);

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(obj);
    ce_cdb_a0->set_uint64(writer, SHADER_PROP, program.idx);
    ce_cdb_a0->write_commit(writer);
}

static void offline(uint64_t name,
                    uint64_t obj) {
    CE_UNUSED(name);

    const uint64_t program = ce_cdb_a0->read_uint64(obj, SHADER_PROP, 0);
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
int shader_init(struct ce_api_a0 *api) {
    _G = (struct _G){.allocator = ce_memory_a0->system};

    ce_api_a0->register_api(RESOURCE_I_NAME, &ct_resource_i0);

    return 1;
}

void shader_shutdown() {
}

ct_render_program_handle_t shader_get(uint64_t shader) {
    const uint64_t idx = ce_cdb_a0->read_uint64(shader, SHADER_PROP, 0);
    return (ct_render_program_handle_t) {.idx=(uint16_t) idx};
}

static struct ct_shader_a0 shader_api = {
        .get = shader_get
};

struct ct_shader_a0 *ct_shader_a0 = &shader_api;

static void _init_api(struct ce_api_a0 *api) {
    api->register_api("ct_shader_a0", &shader_api);
}

CE_MODULE_DEF(
        shader,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ce_os_a0);
            CE_INIT_API(api, ce_log_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ct_renderer_a0);
        },
        {
            CE_UNUSED(reload);
            _init_api(api);
            shader_init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);

            shader_shutdown();
        }
)