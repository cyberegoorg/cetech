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


#include "cetech/machine/machine.h"
#include "cetech/resource/resource.h"
#include <cetech/texture/texture.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/kernel/kernel.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/resource_preview/resource_preview.h>
#include <cetech/resource/resource_compiler.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/ecs/ecs.h>
#include <celib/os/path.h>
#include <celib/os/process.h>
#include <celib/os/vio.h>
#include <celib/containers/hash.h>


//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct _G {
    ce_alloc_t0 *allocator;
    ct_cdb_ev_queue_o0 *changed_obj_queue;
} _G;

typedef struct ct_texture_obj_t {
    const char *input;
    bool gen_mipmaps;
    bool is_normalmap;
    void *texture_data;
    uint64_t texture_handler;
} ct_texture_obj_t;

//==============================================================================
// Compiler private
//==============================================================================


//==============================================================================
// Resource
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

void _texture_resource_online(ce_cdb_t0 db,
                              uint64_t obj) {
    texture_online(db, obj);
}

void _texture_resource_offline(ce_cdb_t0 db,
                               uint64_t obj) {
    texture_offline(db, obj);
}

static int _texturec(const char *input,
                     const char *output,
                     int gen_mipmaps,
                     int is_normalmap) {
    ce_alloc_t0 *alloc = ce_memory_a0->system;
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

    ct_resource_compiler_a0->gen_tmp_file(output_path, CE_ARRAY_LEN(output_path),
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


static bool texture_compiler(ce_cdb_t0 db,
                             uint64_t k) {
    return _compile(db, k);
}

static uint64_t cdb_type() {
    return TEXTURE_TYPE;
}

static void draw_property(ce_cdb_t0 db,
                          uint64_t obj,
                          uint64_t context,
                          const char *filter) {

    ct_editor_ui_a0->prop_filename(obj, "Input", filter, TEXTURE_INPUT, "png,tga,bmp", 0);
    ct_editor_ui_a0->prop_bool(obj, "Gen mipmaps", filter, TEXTURE_GEN_MIPMAPS);
    ct_editor_ui_a0->prop_bool(obj, "Is normalmap", filter, TEXTURE_IS_NORMALMAP);
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

static void draw_raw(uint64_t obj,
                     ce_vec2_t size) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    bgfx_texture_handle_t texture = {
            .idx = (uint16_t) ce_cdb_a0->read_uint64(reader, TEXTURE_HANDLER_PROP, 0)
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

static const char *name() {
    return "texture";
}


static struct ct_resource_i0 ct_resource_api = {
        .name = name,
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
    ce_cdb_prop_ev_t0 ev = {};

    uint64_t *to_compile_obj = NULL;
    ce_hash_t obj_set = {};

    while (ce_cdb_a0->pop_objs_events(_G.changed_obj_queue, &ev)) {
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
        {.name = "input", .type = CE_CDB_TYPE_STR},
        {.name = "gen_mipmaps", .type = CE_CDB_TYPE_BOOL},
        {.name = "is_normalmap", .type = CE_CDB_TYPE_BOOL},
        {.name = "texture_data", .type = CE_CDB_TYPE_BLOB},
        {.name = "texture_handler", .type = CE_CDB_TYPE_UINT64},
};

void CE_MODULE_LOAD(texture)(struct ce_api_a0 *api,
                             int reload) {

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .changed_obj_queue = ce_cdb_a0->new_objs_listener(ce_cdb_a0->db()),
    };

    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_renderer_a0);

    api->add_api(CT_TEXTURE_API, &texture_api, sizeof(texture_api));

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &property_editor_api, sizeof(property_editor_api));

    api->add_impl(CT_KERNEL_TASK_I,
                  &texture_compile_watch_task, sizeof(texture_compile_watch_task));

    api->add_impl(CT_RESOURCE_I, &ct_resource_api, sizeof(ct_resource_api));

    ce_cdb_a0->reg_obj_type(TEXTURE_TYPE, texture_prop, CE_ARRAY_LEN(texture_prop));
}

void CE_MODULE_UNLOAD(texture)(struct ce_api_a0 *api,
                               int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };
}
