//==============================================================================
// Include
//==============================================================================

#include "celib/allocator.h"

#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/log.h"
#include <celib/os.h>
#include "celib/buffer.inl"
#include <celib/module.h>

#include <celib/config.h>
#include <celib/ydb.h>
#include <celib/cdb.h>
#include <celib/ydb.h>
#include <celib/ebus.h>

#include "cetech/machine/machine.h"
#include "cetech/resource/resource.h"
#include <cetech/texture/texture.h>
#include <cetech/renderer/renderer.h>
#include <cetech/debugui/debugui.h>
#include <cetech/kernel/kernel.h>
#include <cetech/asset_editor/sourcedb_ui.h>
#include <cetech/asset_editor/asset_preview.h>
#include <cetech/resource/builddb.h>
#include <cetech/resource/sourcedb.h>
#include <cetech/resource/resource_compiler.h>
#include <cetech/editor/property.h>

//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct _G {
    struct ce_alloc *allocator;
} _G;

//==============================================================================
// Compiler private
//==============================================================================


//==============================================================================
// Resource
//==============================================================================

void _texture_resource_online(uint64_t name,
                              uint64_t obj) {

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(obj);

    uint64_t blob_size = 0;
    void *blob;
    blob = ce_cdb_a0->read_blob(reader, TEXTURE_DATA, &blob_size, 0);

    const ct_render_memory_t *mem = ct_renderer_a0->make_ref(blob, blob_size);

    ct_render_texture_handle_t texture;
    texture = ct_renderer_a0->create_texture(mem,
                                             CT_RENDER_TEXTURE_NONE,
                                             0, NULL);

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(obj);
    ce_cdb_a0->set_uint64(writer, TEXTURE_HANDLER_PROP, texture.idx);
    ce_cdb_a0->write_commit(writer);
}

void _texture_resource_offline(uint64_t name,
                               uint64_t obj) {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(obj);

    const uint64_t texture = ce_cdb_a0->read_uint64(reader, TEXTURE_HANDLER_PROP,
                                                    0);
    ct_renderer_a0->destroy_texture(
            (ct_render_texture_handle_t) {.idx=(uint16_t) texture});
}

static int _texturec(const char *input,
                     const char *output,
                     int gen_mipmaps,
                     int is_normalmap) {
    struct ce_alloc *alloc = ce_memory_a0->system;
    char *buffer = NULL;

    char *texturec = ct_resource_compiler_a0->external_join(alloc, "texturec");

    ce_buffer_printf(&buffer, alloc, "%s", texturec);
    ce_buffer_free(texturec, alloc);

    ce_buffer_printf(&buffer, alloc, " -f %s -o %s", input, output);

    if (gen_mipmaps) {
        ce_buffer_printf(&buffer, alloc, " %s", "--mips");
    }

    if (is_normalmap) {
        ce_buffer_printf(&buffer, alloc, " %s", "--normalmap");
    }

    ce_buffer_printf(&buffer, alloc, " %s", "2>&1");

    int status = ce_os_a0->process->exec(buffer);

    ce_log_a0->info("application", "STATUS %d", status);

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

    int ret = snprintf(tmp_filename, max_len, "%s/%s.ktx",
                       tmp_dirname, ce_os_a0->path->filename(filename));

    ce_buffer_free(tmp_dirname, a);

    return ret;
}

static uint64_t _compile(uint64_t obj) {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(obj);
    const ce_cdb_obj_o *c_reader = ce_cdb_a0->read(ce_config_a0->obj());

    const char *input = ce_cdb_a0->read_str(reader, TEXTURE_INPUT, "");
    bool gen_mipmaps = ce_cdb_a0->read_bool(reader, TEXTURE_GEN_MIPMAPS, false);
    bool is_normalmap = ce_cdb_a0->read_bool(reader, TEXTURE_IS_NORMALMAP, false);

    struct ce_alloc *a = ce_memory_a0->system;

    const char *platform = ce_cdb_a0->read_str(c_reader,
                                               CONFIG_PLATFORM,
                                               "");

    char output_path[1024] = {};
    char tmp_filename[1024] = {};

    const char *source_dir = ce_cdb_a0->read_str(c_reader,
                                                 CONFIG_SRC, "");

    char *tmp_dir = ct_resource_compiler_a0->get_tmp_dir(a, platform);
    char *input_path = NULL;
    ce_os_a0->path->join(&input_path, a, 2, source_dir, input);

    _gen_tmp_name(output_path, tmp_dir, CE_ARRAY_LEN(tmp_filename),
                  input);

    int result = _texturec(input_path, output_path, gen_mipmaps, is_normalmap);
    if (result != 0) {
        return 0;
    }

    struct ce_vio *tmp_file = NULL;
    tmp_file = ce_os_a0->vio->from_file(output_path, VIO_OPEN_READ);

    const uint64_t size = tmp_file->size(tmp_file);
    char *tmp_data = CE_ALLOC(ce_memory_a0->system, char, size + 1);
    tmp_file->read(tmp_file, tmp_data, sizeof(char), size);
    tmp_file->close(tmp_file);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
    ce_cdb_a0->set_blob(w, TEXTURE_DATA, tmp_data, size);
    ce_cdb_a0->write_commit(w);

    return obj;
}


uint64_t texture_compiler(const char *filename,
                          uint64_t k,
                          struct ct_resource_id rid,
                          const char *fullname) {
    const char *input = ce_cdb_a0->read_str(k, TEXTURE_INPUT, "");
    uint64_t output = _compile(k);
    ct_builddb_a0->add_dependency(filename, input);
    return output;
}

static uint64_t cdb_type() {
    return TEXTURE_TYPE;
}

static void draw_property(uint64_t obj) {
    ct_sourcedb_ui_a0->ui_str(obj, TEXTURE_INPUT, "Input", 0);
    ct_sourcedb_ui_a0->ui_bool(obj, TEXTURE_GEN_MIPMAPS, "Gen mipmaps");
    ct_sourcedb_ui_a0->ui_bool(obj, TEXTURE_IS_NORMALMAP, "Is normalmap");

    ct_debugui_a0->Text("Texture preview");
    ct_debugui_a0->NextColumn();

    float size[2] = {64, 64};

    const char* name = ce_cdb_a0->read_str(obj, ASSET_NAME, 0);

    ct_render_texture_handle_t texture;
    texture = ct_texture_a0->get(ce_id_a0->id64(name));
    ct_debugui_a0->Image(texture,
                         size,
                         (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                         (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
    ct_debugui_a0->NextColumn();
}


static struct ct_property_editor_i0 ct_property_editor_i0 = {
        .cdb_type = cdb_type,
        .draw_ui = draw_property,
};


static void tooltip(uint64_t resource) {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(resource);

    struct ct_render_texture_handle texture = {
            .idx = (uint16_t) ce_cdb_a0->read_uint64(reader,
                                                     TEXTURE_HANDLER_PROP, 0)
    };

    ct_debugui_a0->Image(texture,
                         (float[2]) {64.0f, 64.0f},
                         (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                         (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
}

static struct ct_asset_preview_i0 ct_asset_preview_i0 = {
        .tooltip = tooltip,
};


void *get_interface(uint64_t name_hash) {
    if (name_hash == ASSET_PREVIEW) {
        return &ct_asset_preview_i0;
    }

    return NULL;
}

static struct ct_resource_i0 ct_resource_i0 = {
        .cdb_type = cdb_type,
        .get_interface = get_interface,
        .online =_texture_resource_online,
        .offline =_texture_resource_offline,
        .compilator = texture_compiler,
};


//==============================================================================
// Interface
//==============================================================================
int texture_init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ce_api_a0->register_api(RESOURCE_I_NAME, &ct_resource_i0);

    return 1;
}

void texture_shutdown() {
}

ct_render_texture_handle_t texture_get(uint64_t name) {
    struct ct_resource_id rid = {.type = TEXTURE_TYPE, .name = name};
    uint64_t obj = ct_resource_a0->get(rid);

    if (!obj) {
        return (ct_render_texture_handle_t) {.idx = UINT16_MAX};
    }

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(obj);
    struct ct_render_texture_handle texture = {
            .idx = (uint16_t) ce_cdb_a0->read_uint64(reader,
                                                     TEXTURE_HANDLER_PROP, 0)
    };

    return texture;
}

static struct ct_texture_a0 texture_api = {
        .get = texture_get
};

struct ct_texture_a0 *ct_texture_a0 = &texture_api;

static void _init_api(struct ce_api_a0 *api) {
    api->register_api("ct_texture_a0", &texture_api);
    api->register_api(PROPERTY_EDITOR_INTERFACE_NAME, &ct_property_editor_i0);
}


CE_MODULE_DEF(
        texture,
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
            texture_init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);

            texture_shutdown();
        }
)