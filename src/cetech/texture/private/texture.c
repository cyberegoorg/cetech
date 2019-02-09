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
#include <celib/ydb.h>
#include <celib/cdb.h>
#include <celib/ydb.h>


#include "cetech/machine/machine.h"
#include "cetech/resource/resource.h"
#include <cetech/texture/texture.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/kernel/kernel.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/editor/resource_preview.h>
#include <cetech/resource/resourcedb.h>
#include <cetech/resource/resource_compiler.h>
#include <cetech/editor/property.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>
#include <cetech/mesh/primitive_mesh.h>
#include <cetech/material/material.h>
#include <celib/os/path.h>
#include <celib/os/process.h>
#include <celib/os/vio.h>

//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct _G {
    struct ce_alloc_t0 *allocator;
    uint64_t *all_textures;
} _G;

//==============================================================================
// Compiler private
//==============================================================================


//==============================================================================
// Resource
//==============================================================================

void texture_online(uint64_t obj) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    uint64_t blob_size = 0;
    void *blob;
    blob = ce_cdb_a0->read_blob(reader, TEXTURE_DATA, &blob_size, 0);

    const bgfx_memory_t *mem = ct_gfx_a0->bgfx_make_ref(blob, blob_size);

    bgfx_texture_handle_t texture;
    texture = ct_gfx_a0->bgfx_create_texture(mem,
                                             BGFX_TEXTURE_NONE,
                                             0, NULL);

    ce_cdb_obj_o0 *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
    ce_cdb_a0->set_uint64(writer, TEXTURE_HANDLER_PROP, texture.idx);
    ce_cdb_a0->write_commit(writer);

}

void texture_offline(uint64_t obj) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    const uint64_t texture = ce_cdb_a0->read_uint64(reader,
                                                    TEXTURE_HANDLER_PROP,
                                                    0);
    ct_gfx_a0->bgfx_destroy_texture(
            (bgfx_texture_handle_t) {.idx=(uint16_t) texture});
}

void _texture_resource_online(uint64_t name,
                              uint64_t obj) {
    texture_online(obj);
    ce_array_push(_G.all_textures, obj, _G.allocator);
}

void _texture_resource_offline(uint64_t name,
                               uint64_t obj) {
    texture_offline(obj);
}

static int _texturec(const char *input,
                     const char *output,
                     int gen_mipmaps,
                     int is_normalmap) {
    struct ce_alloc_t0 *alloc = ce_memory_a0->system;
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

    int status = ce_os_process_a0->exec(buffer);

    ce_log_a0->info("application", "STATUS %d", status);

    return status;
}

static int _gen_tmp_name(char *tmp_filename,
                         const char *tmp_dir,
                         size_t max_len,
                         const char *filename) {

    struct ce_alloc_t0 *a = ce_memory_a0->system;

    char dir[1024] = {};
    ce_os_path_a0->dir(dir, filename);

    char *tmp_dirname = NULL;
    ce_os_path_a0->join(&tmp_dirname, a, 2, tmp_dir, dir);

    ce_os_path_a0->make_path(tmp_dirname);

    int ret = snprintf(tmp_filename, max_len, "%s/%s.ktx",
                       tmp_dirname, ce_os_path_a0->filename(filename));

    ce_buffer_free(tmp_dirname, a);

    return ret;
}

static bool _compile(struct ce_cdb_t0 db,
                     uint64_t obj) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, obj);
    const ce_cdb_obj_o0 *c_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                   ce_config_a0->obj());

    const char *input = ce_cdb_a0->read_str(reader, TEXTURE_INPUT, "");
    bool gen_mipmaps = ce_cdb_a0->read_bool(reader, TEXTURE_GEN_MIPMAPS, false);
    bool is_normalmap = ce_cdb_a0->read_bool(reader, TEXTURE_IS_NORMALMAP,
                                             false);

    struct ce_alloc_t0 *a = ce_memory_a0->system;

    const char *platform = ce_cdb_a0->read_str(c_reader,
                                               CONFIG_PLATFORM,
                                               "");

    char output_path[1024] = {};
    char tmp_filename[1024] = {};

    const char *source_dir = ce_cdb_a0->read_str(c_reader,
                                                 CONFIG_SRC, "");

    char *tmp_dir = ct_resource_compiler_a0->get_tmp_dir(a, platform);
    char *input_path = NULL;
    ce_os_path_a0->join(&input_path, a, 2, source_dir, input);

    _gen_tmp_name(output_path, tmp_dir, CE_ARRAY_LEN(tmp_filename),
                  input);

    int result = _texturec(input_path, output_path, gen_mipmaps, is_normalmap);
    if (result != 0) {
        return false;
    }

    struct ce_vio *tmp_file = NULL;
    tmp_file = ce_os_vio_a0->from_file(output_path, VIO_OPEN_READ);

    const uint64_t size = tmp_file->size(tmp_file);
    char *tmp_data = CE_ALLOC(ce_memory_a0->system, char, size + 1);
    tmp_file->read(tmp_file, tmp_data, sizeof(char), size);
    tmp_file->close(tmp_file);

    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, obj);
    ce_cdb_a0->set_blob(w, TEXTURE_DATA, tmp_data, size);
    ce_cdb_a0->write_commit(w);

    return true;
}


static bool texture_compiler(struct ce_cdb_t0 db,
                             uint64_t k) {
    return _compile(db, k);
}

static uint64_t cdb_type() {
    return TEXTURE_TYPE;
}

static void draw_property(uint64_t obj) {
    ct_editor_ui_a0->prop_str(obj, TEXTURE_INPUT, "Input", 0);
    ct_editor_ui_a0->prop_bool(obj, TEXTURE_GEN_MIPMAPS, "Gen mipmaps");
    ct_editor_ui_a0->prop_bool(obj, TEXTURE_IS_NORMALMAP, "Is normalmap");

}


static struct ct_property_editor_i0 property_editor_api = {
        .cdb_type = cdb_type,
        .draw_ui = draw_property,
};


static void tooltip(uint64_t resource,
                    ce_vec2_t size) {
//    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(),resource);
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

static void draw_raw(uint64_t obj, ce_vec2_t size) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    bgfx_texture_handle_t texture = {
            .idx = (uint16_t) ce_cdb_a0->read_uint64(reader,
                                                     TEXTURE_HANDLER_PROP, 0)
    };

    ct_debugui_a0->Image(texture,
                         &size,
                         &(ce_vec4_t) {1.0f, 1.0f, 1.0f, 1.0f},
                         &(ce_vec4_t) {0.0f, 0.0f, 0.0, 0.0f});
}

static struct ct_resource_preview_i0 ct_resource_preview_api = {
        .tooltip = tooltip,
        .draw_raw = draw_raw,
};


void *get_interface(uint64_t name_hash) {
    if (name_hash == RESOURCE_PREVIEW_I) {
        return &ct_resource_preview_api;
    }

    return NULL;
}

static const char *display_icon() {
    return ICON_FA_PICTURE_O;
}

static struct ct_resource_i0 ct_resource_api = {
        .cdb_type = cdb_type,
        .display_icon = display_icon,
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

    ce_api_a0->register_api(RESOURCE_I, &ct_resource_api, sizeof(ct_resource_api));

    return 1;
}

void texture_shutdown() {
}

bgfx_texture_handle_t texture_get(uint64_t name) {
    struct ct_resource_id_t0 rid = {.uid = name};
    uint64_t obj = rid.uid;

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


static uint64_t task_name() {
    return TEXTURE_TASK;
}

static uint64_t *update_after(uint64_t *n) {
    static uint64_t a[] = {
            CT_RENDER_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}

static void _update(float dt) {
    uint32_t tn = ce_array_size(_G.all_textures);
    for (int i = 0; i < tn; ++i) {
        uint64_t obj = _G.all_textures[i];

        const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

        uint32_t change_n = 0;
        const struct ce_cdb_change_ev_t0 *changes;
        changes = ce_cdb_a0->changed(reader, &change_n);

        bool compile = false;

        if (change_n) {
            for (int j = 0; j < change_n; ++j) {
                struct ce_cdb_change_ev_t0 ev = changes[j];
                if (ev.prop == TEXTURE_HANDLER_PROP) {
                    compile = false;
                    break;
                }
                compile = true;
            }
        }

        if (compile) {
            texture_offline(obj);
            _compile(ce_cdb_a0->db(), obj);
            texture_online(obj);
        }
    }
}

static struct ct_kernel_task_i0 texture_task = {
        .name = task_name,
        .update = _update,
        .update_after = update_after,
};


static void _init_api(struct ce_api_a0 *api) {
    api->register_api(CT_TEXTURE_API, &texture_api, sizeof(texture_api));
    api->register_api(PROPERTY_EDITOR_INTERFACE, &property_editor_api, sizeof(property_editor_api));
    api->register_api(KERNEL_TASK_INTERFACE, &texture_task, sizeof(texture_task));
}

CE_MODULE_DEF(
        texture,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ct_resource_a0);
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