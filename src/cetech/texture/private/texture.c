//==============================================================================
// Include
//==============================================================================

#include "celib/memory/allocator.h"

#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/log.h"

#include "celib/containers/buffer.h"
#include <celib/module.h>

#include <celib/config.h>
#include <celib/cdb.h>
#include <celib/fs.h>


#include "cetech/machine/machine.h"
#include "cetech/asset/asset.h"
#include <cetech/texture/texture.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>

#include <cetech/kernel/kernel.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/asset_preview/asset_preview.h>
#include <cetech/asset_io/asset_io.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/ui/icons_font_awesome.h>
#include <cetech/ecs/ecs.h>
#include <celib/os/path.h>
#include <celib/os/process.h>
#include <celib/os/vio.h>
#include <celib/containers/hash.h>
#include <cetech/ui/ui.h>

CE_MODULE(ct_assetdb_a0);

//==============================================================================
// GLobals
//==============================================================================

#define _G TextureAssetGlobals
struct _G {
    ce_alloc_t0 *allocator;
} _G;

//==============================================================================
// Compiler private
//==============================================================================


//==============================================================================
// Asset
//==============================================================================

void texture_online(ce_cdb_t0 db,
                    uint64_t obj) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, obj);

    uint64_t blob_size = 0;
    void *blob;
    blob = ce_cdb_a0->read_blob(reader, TEXTURE_DATA, &blob_size, 0);

    const bgfx_memory_t *mem = ct_gfx_a0->bgfx_make_ref(blob, blob_size);

    bgfx_texture_handle_t texture;
    texture = ct_gfx_a0->bgfx_create_texture(mem,
                                             BGFX_TEXTURE_NONE,
                                             0, NULL);

    ce_cdb_obj_o0 *writer = ce_cdb_a0->write_begin(db, obj);
    ce_cdb_a0->set_uint64(writer, TEXTURE_HANDLER_PROP, texture.idx);
    ce_cdb_a0->write_commit(writer);
}

void texture_offline(ce_cdb_t0 db,
                     uint64_t obj) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, obj);

    const uint64_t texture = ce_cdb_a0->read_uint64(reader,
                                                    TEXTURE_HANDLER_PROP,
                                                    0);
    ct_gfx_a0->bgfx_destroy_texture(
            (bgfx_texture_handle_t) {.idx=(uint16_t) texture});
}

void _texture_asset_online(ce_cdb_t0 db,
                           uint64_t obj) {
    texture_online(db, obj);
}

void _texture_asset_offline(ce_cdb_t0 db,
                            uint64_t obj) {
    texture_offline(db, obj);
}

static int _texturec(const char *input,
                     const char *output,
                     int gen_mipmaps,
                     int is_normalmap) {
    ce_alloc_t0 *alloc = ce_memory_a0->system;
    char *buffer = NULL;

    char *texturec = ct_asset_a0->external_join(alloc, "texturec");

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

    int status = ce_os_process_a0->exec(buffer);

    ce_log_a0->info("application", "STATUS %d", status);

    return status;
}


static bool _compile(ce_cdb_t0 db,
                     uint64_t obj) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, obj);

    const char *input = ce_cdb_a0->read_str(reader, TEXTURE_INPUT, "");
    bool gen_mipmaps = ce_cdb_a0->read_bool(reader, TEXTURE_GEN_MIPMAPS, false);
    bool is_normalmap = ce_cdb_a0->read_bool(reader, TEXTURE_IS_NORMALMAP, false);

    ce_alloc_t0 *a = ce_memory_a0->system;

    const char *platform = ce_config_a0->read_str(CONFIG_PLATFORM, "");

    char output_path[1024] = {};

    const char *source_dir = ce_config_a0->read_str(CONFIG_SRC, "");

    char *input_path = NULL;
    ce_os_path_a0->join(&input_path, a, 2, source_dir, input);

    ct_asset_a0->gen_tmp_file(output_path, CE_ARRAY_LEN(output_path),
                              platform, input, "ktx");

    int result = _texturec(input_path, output_path, gen_mipmaps, is_normalmap);
    if (result != 0) {
        return false;
    }

    ce_vio_t0 *tmp_file = NULL;
    tmp_file = ce_os_vio_a0->from_file(output_path, VIO_OPEN_READ);

    const uint64_t size = tmp_file->vt->size(tmp_file->inst);
    char *tmp_data = CE_ALLOC(ce_memory_a0->system, char, size + 1);
    tmp_file->vt->read(tmp_file->inst, tmp_data, sizeof(char), size);
    ce_os_vio_a0->close(tmp_file);

    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, obj);
    ce_cdb_a0->set_blob(w, TEXTURE_DATA, tmp_data, size);
    ce_cdb_a0->write_commit(w);
    CE_FREE(ce_memory_a0->system, tmp_data);

    return true;
}

static uint64_t cdb_type() {
    return TEXTURE_TYPE;
}


////
static bool _import(ce_cdb_t0 db,
                    uint64_t dcc_obj) {
    {
        const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, dcc_obj);
        uint32_t assets_n = ce_cdb_a0->read_objset_num(r, CT_DCC_ASSET_ASSETS_PROP);
        if (!assets_n) {
            uint64_t dcc_texture = ce_cdb_a0->create_object(db, CT_DCC_ASSET_TEXTURE_TYPE);
            ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, dcc_texture);
            ce_cdb_a0->set_uint64(w, CT_DCC_ASSET_UID_PROP, ce_cdb_a0->gen_uid(db).id);
            ce_cdb_a0->write_commit(w);

            w = ce_cdb_a0->write_begin(db, dcc_obj);
            ce_cdb_a0->objset_add_obj(w, CT_DCC_ASSET_ASSETS_PROP, dcc_texture);
            ce_cdb_a0->write_commit(w);
        }
    }

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, dcc_obj);
    uint32_t assets_n = ce_cdb_a0->read_objset_num(r, CT_DCC_ASSET_ASSETS_PROP);
    uint64_t assets[assets_n];
    ce_cdb_a0->read_objset(r, CT_DCC_ASSET_ASSETS_PROP, assets);

    const ce_cdb_obj_o0 *dcc_texture_r = ce_cdb_a0->read(db, assets[0]);

    uint64_t obj_uid = ce_cdb_a0->read_uint64(dcc_texture_r, CT_DCC_ASSET_UID_PROP, 0);
    uint64_t obj = ce_cdb_a0->create_object_uid(db, (ce_cdb_uuid_t0) {obj_uid}, TEXTURE_TYPE, true);

    const char *input = ce_cdb_a0->read_str(r, CT_DCC_FILENAME_PROP, NULL);
    bool gen_mipmaps = ce_cdb_a0->read_bool(dcc_texture_r, TEXTURE_GEN_MIPMAPS, false);
    bool is_normalmap = ce_cdb_a0->read_bool(dcc_texture_r, TEXTURE_IS_NORMALMAP, false);

    ce_alloc_t0 *a = ce_memory_a0->system;

    const char *platform = ce_config_a0->read_str(CONFIG_PLATFORM, "");

    char output_path[1024] = {};

    const char *source_dir = ce_config_a0->read_str(CONFIG_SRC, "");

    char *input_path = NULL;
    ce_os_path_a0->join(&input_path, a, 2, source_dir, input);

    ct_asset_a0->gen_tmp_file(output_path, CE_ARRAY_LEN(output_path),
                              platform, input, "ktx");

    int result = _texturec(input_path, output_path, gen_mipmaps, is_normalmap);
    if (result != 0) {
        return false;
    }

    ce_vio_t0 *tmp_file = NULL;
    tmp_file = ce_os_vio_a0->from_file(output_path, VIO_OPEN_READ);

    const uint64_t size = tmp_file->vt->size(tmp_file->inst);
    char *tmp_data = CE_ALLOC(ce_memory_a0->system, char, size + 1);
    tmp_file->vt->read(tmp_file->inst, tmp_data, sizeof(char), size);
    ce_os_vio_a0->close(tmp_file);

    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, obj);
    ce_cdb_a0->set_blob(w, TEXTURE_DATA, tmp_data, size);
    ce_cdb_a0->write_commit(w);
    CE_FREE(ce_memory_a0->system, tmp_data);

    char texture_filename[512];
    snprintf(texture_filename, CE_ARRAY_LEN(texture_filename), "%s.texture", input);

//    int64_t mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, texture_filename);
//    ct_assetdb_a0->put_file(texture_filename, mtime);
    ct_asset_a0->save_to_cdb(db, obj, texture_filename);
    return true;
}

static const ce_cdb_prop_def_t0 dcc_asset_textue_prop[] = {
        {.name = "uuid", .type = CE_CDB_TYPE_UINT64},
        {.name = "gen_mipmaps", .type = CE_CDB_TYPE_BOOL, .value.b = true},
        {.name = "is_normalmap", .type = CE_CDB_TYPE_BOOL},
};

static bool supported_extension(const char *extension) {
    return !strcmp(extension, "png")
           || !strcmp(extension, "tga")
           || !strcmp(extension, "jpg")
           || !strcmp(extension, "jpeg");
}

static ct_asset_dcc_io_i0 texture_io = {
        .import_group = CT_TEXTURE_IMPORT_GROUP,
        .supported_extension = supported_extension,
        .import_dcc = _import,
};

////
static void draw_property(ce_cdb_t0 db,
                          uint64_t obj,
                          uint64_t context,
                          const char *filter) {

    ct_editor_ui_a0->prop_filename(obj, "Input", TEXTURE_INPUT, "png,tga,bmp", 0);
    ct_editor_ui_a0->prop_bool(obj, "Gen mipmaps", TEXTURE_GEN_MIPMAPS);
    ct_editor_ui_a0->prop_bool(obj, "Is normalmap", TEXTURE_IS_NORMALMAP);
}

static void tooltip(uint64_t asset,
                    ce_vec2_t size) {
//    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(),asset);
//
//    bgfx_texture_handle_t texture = {
//            .idx = (uint16_t) ce_cdb_a0->read_uint64(reader,
//                                                     TEXTURE_HANDLER_PROP, 0)
//    };
//
//    ct_debugui_a0->Image(texture,
//                         size,
//                         (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
//                         (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
}

static void draw_raw(uint64_t obj,
                     ce_vec2_t size) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    ct_ui_a0->image(&(struct ct_ui_image_t0) {
            .user_texture_id = ce_cdb_a0->read_uint64(reader, TEXTURE_HANDLER_PROP, 0),
            .size = size,
            .tint_col = {1.0f, 1.0f, 1.0f, 1.0f},
            .border_col ={0.0f, 0.0f, 0.0, 0.0f},
    });

}

static struct ct_asset_preview_i0 ct_asset_preview_api = {
        .tooltip = tooltip,
        .draw_raw = draw_raw,
};


static const char *display_icon() {
    return ICON_FA_PICTURE_O;
}

static const char *name() {
    return "texture";
}


static struct ct_asset_i0 ct_asset_api = {
        .name = name,
        .cdb_type = cdb_type,
        .display_icon = display_icon,
        .online =_texture_asset_online,
        .offline =_texture_asset_offline,
};


//==============================================================================
// Interface
//==============================================================================
bgfx_texture_handle_t texture_get(uint64_t name) {
    ce_cdb_uuid_t0 rid = {.id = name};
    uint64_t obj = rid.id;

    if (!obj) {
        return (bgfx_texture_handle_t) {.idx = UINT16_MAX};
    }

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    bgfx_texture_handle_t texture = {
            .idx = (uint16_t) ce_cdb_a0->read_uint64(reader,
                                                     TEXTURE_HANDLER_PROP, 0)
    };

    return texture;
}

static struct ct_texture_a0 texture_api = {
        .get = texture_get
};

struct ct_texture_a0 *ct_texture_a0 = &texture_api;

static void compile_watch(float dt) {
    uint64_t *to_compile_obj = NULL;
    ce_hash_t obj_set = {};

    uint32_t changes_n = 0;
    const ce_cdb_prop_ev_t0 *objs_evs = ce_cdb_a0->objs_changes(ce_cdb_a0->db(), &changes_n);

    for (uint32_t i = 0; i < changes_n; ++i) {
        ce_cdb_prop_ev_t0 ev = objs_evs[i];

        if (ce_cdb_a0->obj_type(ce_cdb_a0->db(), ev.obj) != TEXTURE_TYPE) {
            continue;
        }

        if (ev.prop == TEXTURE_DATA) {
            continue;
        }

        if (ev.prop == TEXTURE_HANDLER_PROP) {
            continue;
        }


        if (!ce_hash_contain(&obj_set, ev.obj)) {
            ce_array_push(to_compile_obj, ev.obj, _G.allocator);
            ce_hash_add(&obj_set, ev.obj, ev.obj, _G.allocator);
        }
    }

    uint32_t n = ce_array_size(to_compile_obj);
    for (int i = 0; i < n; ++i) {
        uint64_t obj = to_compile_obj[i];
        texture_offline(ce_cdb_a0->db(), obj);
        _compile(ce_cdb_a0->db(), obj);
        texture_online(ce_cdb_a0->db(), obj);
    }

    ce_hash_free(&obj_set, _G.allocator);
    ce_array_free(to_compile_obj, _G.allocator);
}

static struct ct_kernel_task_i0 texture_compile_watch_task = {
        .name = TEXTURE_TASK,
        .update = compile_watch,
        .update_after = CT_KERNEL_AFTER(CT_RENDER_TASK),
};


static const ce_cdb_prop_def_t0 texture_prop[] = {
        {.name = "texture_data", .type = CE_CDB_TYPE_BLOB},
        {.name = "texture_handler", .type = CE_CDB_TYPE_UINT64},
};

void CE_MODULE_LOAD(texture)(struct ce_api_a0 *api,
                             int reload) {

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ce_id_a0->id64("texture");

    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_asset_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_renderer_a0);

    api->add_api(CT_TEXTURE_A0_STR, &texture_api, sizeof(texture_api));

    api->add_impl(CT_KERNEL_TASK_I0_STR,
                  &texture_compile_watch_task, sizeof(texture_compile_watch_task));

    api->add_impl(CT_ASSET_I0_STR, &ct_asset_api, sizeof(ct_asset_api));

    api->add_impl(CT_DCC_ASSET_IO_I0_STR, &texture_io, sizeof(texture_io));

    ce_cdb_a0->reg_obj_type(TEXTURE_TYPE, texture_prop, CE_ARRAY_LEN(texture_prop));
    ce_cdb_a0->set_aspect(TEXTURE_TYPE, CT_PROPERTY_EDITOR_ASPECT, draw_property);
    ce_cdb_a0->set_aspect(TEXTURE_TYPE, CT_PREVIEW_ASPECT, &ct_asset_preview_api);

    ce_cdb_a0->reg_obj_type(CT_DCC_ASSET_TEXTURE_TYPE,
                            dcc_asset_textue_prop,
                            CE_ARRAY_LEN(dcc_asset_textue_prop));

    ct_asset_io_a0->create_import_group(CT_TEXTURE_IMPORT_GROUP, 2.0f);
}

void CE_MODULE_UNLOAD(texture)(struct ce_api_a0 *api,
                               int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };
}
