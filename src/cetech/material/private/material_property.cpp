//==============================================================================
// Include
//==============================================================================

#include "corelib/allocator.h"

#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/log.h"
#include <corelib/os.h>
#include "cetech/resource/resource.h"
#include "material.h"
#include <corelib/module.h>
#include <cetech/playground//asset_property.h>
#include <cetech/debugui/debugui.h>
#include <cetech/texture/texture.h>
#include <corelib/ydb.h>
#include <cetech/playground/command_system.h>
#include <cetech/material/material.h>
#include <cstdio>
#include <cstring>
#include <corelib/macros.h>


//==============================================================================
// GLobals
//==============================================================================

#define _G MaterialPropGlobals
static struct _G {
    ct_alloc *allocator;
} _G;


static void ui_vec4(uint64_t var) {
    const char *str;
    str = ct_cdb_a0->read_str(var, MATERIAL_VAR_NAME_PROP, "");

    float v[4] = {0.0f};
    ct_cdb_a0->read_vec4(var, MATERIAL_VAR_VALUE_PROP, v);

    if (ct_debugui_a0->DragFloat3(str, v, 0.1f, -1.0f, 1.0f, "%.5f", 1.0f)) {
        ct_cdb_obj_o *wr = ct_cdb_a0->write_begin(var);
        ct_cdb_a0->set_vec4(wr, MATERIAL_VAR_VALUE_PROP, v);
        ct_cdb_a0->write_commit(wr);
    }
}

static void ui_color4(uint64_t var) {
    const char *str;
    str = ct_cdb_a0->read_str(var, MATERIAL_VAR_NAME_PROP, "");

    float v[4] = {0.0f};
    ct_cdb_a0->read_vec4(var, MATERIAL_VAR_VALUE_PROP, v);

    ct_debugui_a0->ColorEdit4(str, v, true);

    ct_cdb_obj_o *wr = ct_cdb_a0->write_begin(var);
    ct_cdb_a0->set_vec4(wr, MATERIAL_VAR_VALUE_PROP, v);
    ct_cdb_a0->write_commit(wr);

//    if (ct_debugui_a0->ColorEdit3(str, v)) {
//        uint64_t  wr = ct_cdb_a0->write_begin(var);
//        ct_cdb_a0->set_vec4(wr, MATERIAL_VAR_VALUE_PROP, v);
//        ct_cdb_a0->write_commit(wr);
//    }
}

static void ui_texture(uint64_t variable) {
    const char *str;
    str = ct_cdb_a0->read_str(variable, MATERIAL_VAR_NAME_PROP, "");

    uint64_t name = ct_cdb_a0->read_uint64(variable, MATERIAL_VAR_VALUE_PROP,
                                           0);


    char buff[128];
    snprintf(buff, 128, "%llu", name);

    ct_debugui_a0->InputText(str, buff, strlen(buff),
                             DebugInputTextFlags_ReadOnly, 0, NULL);
    float size[2];
    ct_debugui_a0->GetWindowSize(size);
    size[1] = size[0];

    ct_debugui_a0->Image2(ct_texture_a0->get(name),
                          size,
                          (float[2]) {0.0f, 0.0f},
                          (float[2]) {1.0f, 1.0f},
                          (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                          (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
}

static void material_asset(struct ct_resource_id asset,
                           const char *path) {
    uint64_t material = ct_resource_a0->get(asset);

    uint64_t layer_count = ct_cdb_a0->prop_count(material);
    uint64_t layer_keys[layer_count];
    ct_cdb_a0->prop_keys(material, layer_keys);


    for (int i = 0; i < layer_count; ++i) {
        if (layer_keys[i] == CT_ID64_0("asset_name")) {
            continue;
        }

        if (ct_debugui_a0->CollapsingHeader("Layer",
                                            DebugUITreeNodeFlags_DefaultOpen)) {
            uint64_t layer;
            layer = ct_cdb_a0->read_ref(material, layer_keys[i], 0);

            uint64_t variables;
            variables = ct_cdb_a0->read_ref(layer, MATERIAL_VARIABLES_PROP, 0);

            uint64_t count = ct_cdb_a0->prop_count(variables);
            uint64_t keys[count];
            ct_cdb_a0->prop_keys(variables, keys);

            for (int j = 0; j < count; ++j) {
                uint64_t var;
                var = ct_cdb_a0->read_ref(variables, keys[j], 0);

                uint64_t var_type;
                var_type = ct_cdb_a0->read_uint64(var, MATERIAL_VAR_TYPE_PROP,
                                                  0);

                switch (var_type) {
                    case MAT_VAR_NONE:
                        break;

                    case MAT_VAR_INT:
                        break;

                    case MAT_VAR_TEXTURE:
                        ui_texture(var);
                        break;

                    case MAT_VAR_TEXTURE_HANDLER:
                        ui_texture(var);
                        break;

                    case MAT_VAR_VEC4:
                        ui_vec4(var);
                        break;

                    case MAT_VAR_COLOR4:
                        ui_color4(var);
                        break;

                    case MAT_VAR_MAT44:
                        break;

                    default:
                        break;
                }
            }
        }
    }
}


static int _init(struct ct_api_a0 *api) {
    CT_UNUSED(api);

    _G = {
            .allocator = ct_memory_a0->main_allocator()
    };

    ct_asset_property_a0->register_asset(CT_ID32_0("material"), material_asset);

    return 1;
}

static void _shutdown() {
    _G = {};
}


CETECH_MODULE_DEF(
        material_property,
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
            CETECH_GET_API(api, ct_cdb_a0);
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