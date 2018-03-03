 //==============================================================================
// Include
//==============================================================================

#include "cetech/core/memory/allocator.h"
#include "cetech/core/containers/map.inl"
#include "cetech/core/containers/buffer.h"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/log/log.h"
#include "cetech/core/os/path.h"
#include "cetech/core/os/vio.h"
#include "cetech/engine/resource/resource.h"
#include <cetech/core/module/module.h>
#include <cetech/playground//asset_property.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/renderer/texture.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/playground/command_system.h>

using namespace celib;

//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
static struct _G {
} _G;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_asset_property_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_texture_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_cmd_system_a0);


static void set_ydb_bool_cmd(const struct ct_cmd *cmd,
                             bool inverse) {
    const struct ct_ydb_cmd_bool_s *bool_cmd = (const ct_ydb_cmd_bool_s *) cmd;

    bool value = inverse ? bool_cmd->old_value : bool_cmd->new_value;

    ct_ydb_a0.set_bool(bool_cmd->ydb.filename,
                       bool_cmd->ydb.keys,
                       bool_cmd->ydb.keys_count, value);

    ct_resource_a0.compile_and_reload(bool_cmd->ydb.filename);
}

static void ui_input(const char *path) {
    uint64_t tmp_keys[32] = {};

    tmp_keys[0] = ct_yng_a0.key("input");

    const char *input = ct_ydb_a0.get_string(path, tmp_keys, 1, "");

    static char txt_buffer[128] = {};
    strcpy(txt_buffer, input);

    ct_debugui_a0.InputText("input", txt_buffer, CT_ARRAY_LEN(txt_buffer),
                            DebugInputTextFlags_ReadOnly, 0, NULL);
}

static void cmd_description(char *buffer,
                            uint32_t buffer_size,
                            const struct ct_cmd *cmd,
                            bool inverse) {
    static const uint64_t set_mipmaps = CT_ID64_0(
            "texture_set_mipmaps");
    static const uint64_t set_normalmap = CT_ID64_0(
            "texture_set_normalmap");

    if (cmd->type == set_mipmaps) {
        struct ct_ydb_cmd_bool_s *ydb_cmd = (struct ct_ydb_cmd_bool_s *) cmd;
        snprintf(buffer, buffer_size, "%s texture mipmaps",
                 ydb_cmd->new_value ? "Enable" : "Disable");
    } else if (cmd->type == set_normalmap) {
        struct ct_ydb_cmd_bool_s *ydb_cmd = (struct ct_ydb_cmd_bool_s *) cmd;
        snprintf(buffer, buffer_size, "%s texture normalmap",
                 ydb_cmd->new_value ? "Enable" : "Disable");
    }
}

static void ui_gen_mipmaps(const char *path) {
    uint64_t tmp_keys = ct_yng_a0.key("gen_mipmaps");

    bool gen_mipmaps = ct_ydb_a0.get_bool(path, &tmp_keys, 1, false);
    bool new_gen_mipmaps = gen_mipmaps;

    if (ct_debugui_a0.Checkbox("gen mipmaps", &new_gen_mipmaps)) {
        struct ct_ydb_cmd_bool_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ydb_cmd_bool_s),
                        .type = CT_ID64_0("texture_set_mipmaps"),
                },

                .ydb = {
                        .filename = path,
                        .keys = {[0] = tmp_keys},
                        .keys_count = 1,
                },

                .new_value = new_gen_mipmaps,
                .old_value = gen_mipmaps,
        };
        ct_cmd_system_a0.execute(&cmd.header);
    }
}

static void ui_is_normalmap(const char *path) {
    uint64_t tmp_keys = ct_yng_a0.key("is_normalmap");

    bool is_normalmap = ct_ydb_a0.get_bool(path, &tmp_keys, 1, false);
    bool new_is_normalmap = is_normalmap;

    if (ct_debugui_a0.Checkbox("is normalmap", &new_is_normalmap)) {
        struct ct_ydb_cmd_bool_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ydb_cmd_bool_s),
                        .type = CT_ID64_0("texture_set_normalmap"),
                },

                .ydb = {
                        .filename = path,
                        .keys = {[0] = tmp_keys},
                        .keys_count = 1,
                },

                .new_value = new_is_normalmap,
                .old_value = is_normalmap,
        };

        ct_cmd_system_a0.execute(&cmd.header);
    }
}

static void ui_texture_preview(uint32_t name) {
    float size[2];
    ct_debugui_a0.GetWindowSize(size);
    size[1] = size[0];

    ct_debugui_a0.Image2(ct_texture_a0.get(name),
                         size,
                         (float[2]) {0.0f, 0.0f},
                         (float[2]) {1.0f, 1.0f},
                         (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                         (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
}

static void texture_asset(ct_resource_id asset, const char *path) {

    if (ct_debugui_a0.CollapsingHeader("Texture",
                                       DebugUITreeNodeFlags_DefaultOpen)) {
        ui_input(path);
        ui_gen_mipmaps(path);
        ui_is_normalmap(path);
        ui_texture_preview(asset.name);
    }
}

static int _init(ct_api_a0 *api) {
    CT_UNUSED(api);

    _G = {};

    ct_asset_property_a0.register_asset(
            CT_ID32_0("texture"),
            texture_asset);

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("texture_set_mipmaps"),
            (struct ct_cmd_fce) {.execute = set_ydb_bool_cmd, .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("texture_set_normalmap"),
            (struct ct_cmd_fce) {.execute = set_ydb_bool_cmd, .description = cmd_description});

    return 1;
}

static void _shutdown() {
    _G = {};
}


CETECH_MODULE_DEF(
        texture_property,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_asset_property_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_texture_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_cmd_system_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            _shutdown();
        }
)