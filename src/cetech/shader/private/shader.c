//==============================================================================
// Include
//==============================================================================

#include <stdio.h>

#include "celib/memory/allocator.h"
#include "celib/celib_types.h"
#include "celib/macros.h"
#include "celib/containers/buffer.h"

#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/log.h"

#include "celib/id.h"
#include "cetech/machine/machine.h"

#include "cetech/resource/resource.h"

#include <cetech/renderer/gfx.h>

#include <celib/module.h>
#include <cetech/shader/shader.h>
#include <cetech/renderer/renderer.h>


#include <celib/ydb.h>
#include <celib/config.h>
#include <celib/ydb.h>
#include <cetech/kernel/kernel.h>
#include <cetech/resource/resourcedb.h>
#include <cetech/resource/resource_compiler.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <celib/fs.h>
#include <celib/os/path.h>
#include <celib/os/process.h>
#include <celib/os/vio.h>

//==============================================================================
// GLobals
//==============================================================================

#define _G ShaderResourceGlobals
struct _G {
    ce_alloc_t0 *allocator;
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
    ce_alloc_t0 *a = ce_memory_a0->system;

    char *buffer = NULL;

    char *shaderc = ct_resource_compiler_a0->external_join(a, "shaderc");

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

    int status = ce_os_process_a0->exec(buffer);

    ce_log_a0->debug("shaderc", "STATUS %d", status);

    return status;
}

static int _gen_tmp_name(char *tmp_filename,
                         const char *tmp_dir,
                         size_t max_len,
                         const char *filename) {

    ce_alloc_t0 *a = ce_memory_a0->system;

    char dir[1024] = {};
    ce_os_path_a0->dir(dir, filename);

    char *tmp_dirname = NULL;
    ce_os_path_a0->join(&tmp_dirname, a, 2, tmp_dir, dir);

    ce_os_path_a0->make_path(tmp_dirname);

    int ret = snprintf(tmp_filename, max_len, "%s/%s.shaderc", tmp_dirname,
                       ce_os_path_a0->filename(filename));
    ce_buffer_free(tmp_dirname, a);

    return ret;
}


#if CE_PLATFORM_LINUX
const char *platform = "linux";
const char *vs_profile = "120";
const char *fs_profile = "120";
#elif CE_PLATFORM_OSX
//const char *platform = "osx";
const char *vs_profile = "metal";
const char *fs_profile = "metal";
//const char *vs_profile = "120";
//const char *fs_profile = "120";
#elif CE_PLATFORM_WINDOWS
const char* platform = "windows";
const char* vs_profile = "vs_4_0";
const char* fs_profile = "ps_4_0";
#endif

static bool _compile(ce_cdb_t0 db,
                     uint64_t obj) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, obj);

    const char *vs_input = ce_cdb_a0->read_str(reader, SHADER_VS_INPUT, "");
    const char *fs_input = ce_cdb_a0->read_str(reader, SHADER_FS_INPUT, "");

    ce_alloc_t0 *a = ce_memory_a0->system;

    const ce_cdb_obj_o0 *c_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                    ce_config_a0->obj());
    const char *source_dir = ce_cdb_a0->read_str(c_reader,
                                                 CONFIG_SRC, "");
    const char *core_dir = ce_cdb_a0->read_str(c_reader, CONFIG_CORE,
                                               "");

    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, obj);

    char *include_dir = NULL;
    ce_os_path_a0->join(&include_dir, a, 2, core_dir, "bgfxshaders");

    // TODO: temp ce_alloc_t0?
    char output_path[1024] = {};
    char tmp_filename[1024] = {};


    const char *platform;
    platform = ce_cdb_a0->read_str(c_reader, CONFIG_PLATFORM, "");

    char *tmp_dir = ct_resource_compiler_a0->get_tmp_dir(a, platform);

    //////// VS
//    compilator_api->add_dependency(filename, vs_input);

    char *input_path = NULL;
    ce_os_path_a0->join(&input_path, a, 2, source_dir, vs_input);

    _gen_tmp_name(output_path, tmp_dir,
                  CE_ARRAY_LEN(tmp_filename), vs_input);

    int result = _shaderc(input_path, output_path, include_dir, "vertex",
                          platform, vs_profile);

    ce_buffer_free(input_path, a);

    if (result != 0) {
        ce_buffer_free(include_dir, a);
        return false;
    }


    ce_vio_t0 *tmp_file;

    do {
        tmp_file = ce_os_vio_a0->from_file(output_path, VIO_OPEN_READ);
    } while (tmp_file == NULL);


    char *vs_data = CE_ALLOC(ce_memory_a0->system, char,
                             tmp_file->vt->size(tmp_file->inst) + 1);

    uint64_t vs_data_size = tmp_file->vt->size(tmp_file->inst);
    tmp_file->vt->read(tmp_file->inst, vs_data, sizeof(char), vs_data_size);
    ce_os_vio_a0->close(tmp_file);

    ce_cdb_a0->set_blob(w, SHADER_VS_DATA, vs_data, vs_data_size);

    ///////

    //////// FS
//    compilator_api->add_dependency(filename, fs_input);
    ce_buffer_clear(input_path);

    ce_os_path_a0->join(&input_path, a, 2, source_dir, fs_input);

    _gen_tmp_name(output_path, tmp_dir, CE_ARRAY_LEN(tmp_filename),
                  fs_input);

    result = _shaderc(input_path, output_path,
                      include_dir, "fragment",
                      platform, fs_profile);

    ce_buffer_free(input_path, a);

    if (result != 0) {
        ce_buffer_free(include_dir, a);
        return false;
    }

    tmp_file = ce_os_vio_a0->from_file(output_path, VIO_OPEN_READ);
    char *fs_data = CE_ALLOC(ce_memory_a0->system, char,
                             tmp_file->vt->size(tmp_file->inst) + 1);

    uint64_t fs_data_size = tmp_file->vt->size(tmp_file->inst);
    tmp_file->vt->read(tmp_file->inst, fs_data, sizeof(char), fs_data_size);
    ce_os_vio_a0->close(tmp_file);

    ce_cdb_a0->set_blob(w, SHADER_FS_DATA, fs_data, fs_data_size);
    ce_cdb_a0->write_commit(w);

    CE_FREE(a, vs_data);
    CE_FREE(a, fs_data);
    ce_buffer_free(include_dir, a);

    return true;
}

bool shader_compiler(ce_cdb_t0 db,
                     uint64_t k) {
    return _compile(db, k);
}


static void online(uint64_t name,
                   uint64_t obj) {
//    ce_cdb_a0->register_notify(obj, _on_obj_change, NULL);
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    uint64_t fs_blob_size = 0;
    void *fs_blob;
    fs_blob = ce_cdb_a0->read_blob(reader, SHADER_FS_DATA, &fs_blob_size, 0);

    uint64_t vs_blob_size = 0;
    void *vs_blob;
    vs_blob = ce_cdb_a0->read_blob(reader, SHADER_VS_DATA, &vs_blob_size, 0);

    const bgfx_memory_t *vs_mem = ct_gfx_a0->bgfx_make_ref(vs_blob,
                                                           vs_blob_size);
    const bgfx_memory_t *fs_mem = ct_gfx_a0->bgfx_make_ref(fs_blob,
                                                           fs_blob_size);

    bgfx_shader_handle_t vs_shader = ct_gfx_a0->bgfx_create_shader(vs_mem);
    bgfx_shader_handle_t fs_shader = ct_gfx_a0->bgfx_create_shader(fs_mem);

    bgfx_program_handle_t program;
    program = ct_gfx_a0->bgfx_create_program(vs_shader, fs_shader, true);

    ce_cdb_obj_o0 *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
    ce_cdb_a0->set_uint64(writer, SHADER_PROP, program.idx);
    ce_cdb_a0->write_commit(writer);
}

static void offline(uint64_t name,
                    uint64_t obj) {
    CE_UNUSED(name);

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    const uint64_t program = ce_cdb_a0->read_uint64(reader, SHADER_PROP, 0);
    ct_gfx_a0->bgfx_destroy_program(
            (bgfx_program_handle_t) {.idx=(uint16_t) program});
}


static uint64_t cdb_type() {
    return SHADER_TYPE;
}

static const char *display_icon() {
    return ICON_FA_COG;
}

static struct ct_resource_i0 ct_resource_api = {
        .cdb_type = cdb_type,
        .display_icon = display_icon,
        .online = online,
        .offline = offline,
        .compilator = shader_compiler,
};

//==============================================================================
// Interface
//==============================================================================

bgfx_program_handle_t shader_get(uint64_t shader) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), shader);

    const uint64_t idx = ce_cdb_a0->read_uint64(reader, SHADER_PROP, 0);
    return (bgfx_program_handle_t) {.idx=(uint16_t) idx};
}

static struct ct_shader_a0 shader_api = {
        .get = shader_get
};

struct ct_shader_a0 *ct_shader_a0 = &shader_api;


void CE_MODULE_LOAD(shader)(struct ce_api_a0 *api,
                            int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_renderer_a0);

    _G = (struct _G) {.allocator = ce_memory_a0->system};

    api->register_api(CT_SHADER_API, &shader_api, sizeof(shader_api));
    ce_api_a0->register_api(RESOURCE_I, &ct_resource_api, sizeof(ct_resource_api));

}

void CE_MODULE_UNLOAD(shader)(struct ce_api_a0 *api,
                              int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
