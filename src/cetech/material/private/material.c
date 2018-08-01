//==============================================================================
// Include
//==============================================================================
#include <stdio.h>
#include <string.h>

#include "corelib/allocator.h"

#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"


#include "cetech/resource/resource.h"

#include <cetech/renderer/renderer.h>
#include <cetech/material/material.h>
#include <cetech/shader/shader.h>

#include <cetech/texture/texture.h>
#include <corelib/module.h>

#include <corelib/os.h>
#include <corelib/macros.h>
#include <cetech/asset_property/asset_property.h>
#include <cetech/debugui/debugui.h>
#include <cetech/ecs/ecs.h>
#include <cetech/mesh_renderer/mesh_renderer.h>
#include <cetech/asset_preview/asset_preview.h>
#include <cetech/editor_ui/editor_ui.h>

#include "material.h"

int materialcompiler_init(struct ct_api_a0 *api);

//==============================================================================
// Defines
//==============================================================================

#define _G MaterialGlobals
#define LOG_WHERE "material"


//==============================================================================
// GLobals
//==============================================================================

static struct _G {
    struct ct_cdb_t db;
    struct ct_alloc *allocator;
} _G;


//==============================================================================
// Resource
//==============================================================================

static ct_render_uniform_type_t _type_to_bgfx[] = {
        [MAT_VAR_NONE] = CT_RENDER_UNIFORM_TYPE_COUNT,
        [MAT_VAR_INT] = CT_RENDER_UNIFORM_TYPE_INT1,
        [MAT_VAR_TEXTURE] = CT_RENDER_UNIFORM_TYPE_INT1,
        [MAT_VAR_TEXTURE_HANDLER] = CT_RENDER_UNIFORM_TYPE_INT1,
        [MAT_VAR_VEC4] = CT_RENDER_UNIFORM_TYPE_VEC4,
        [MAT_VAR_MAT44] = CT_RENDER_UNIFORM_TYPE_MAT4,
};

static void online(uint64_t name,
                   struct ct_vio *input,
                   uint64_t obj) {
    CT_UNUSED(name);

    const uint64_t size = input->size(input);
    char *data = CT_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

    ct_cdb_a0->load(ct_cdb_a0->db(), data, obj, _G.allocator);

    uint64_t layers_obj = ct_cdb_a0->read_subobject(obj, MATERIAL_LAYERS, 0);

    const uint64_t layers_n = ct_cdb_a0->prop_count(layers_obj);
    uint64_t layers_keys[layers_n];
    ct_cdb_a0->prop_keys(layers_obj, layers_keys);

    for (int i = 0; i < layers_n; ++i) {
        uint64_t layer_obj = ct_cdb_a0->read_subobject(layers_obj,
                                                       layers_keys[i], 0);

        uint64_t variables_obj = ct_cdb_a0->read_subobject(layer_obj,
                                                           MATERIAL_VARIABLES_PROP,
                                                           0);
        const uint64_t variables_n = ct_cdb_a0->prop_count(variables_obj);
        uint64_t variables_keys[variables_n];
        ct_cdb_a0->prop_keys(variables_obj, variables_keys);

        for (int k = 0; k < variables_n; ++k) {
            uint64_t var_obj = ct_cdb_a0->read_subobject(variables_obj,
                                                         variables_keys[k], 0);

            const char *uniform_name = ct_cdb_a0->read_str(var_obj,
                                                           MATERIAL_VAR_NAME_PROP,
                                                           0);
            uint64_t type = ct_cdb_a0->read_uint64(var_obj,
                                                   MATERIAL_VAR_TYPE_PROP, 0);

            const ct_render_uniform_handle_t handler = \
                ct_renderer_a0->create_uniform(uniform_name,
                                               _type_to_bgfx[type], 1);

            ct_cdb_obj_o *var_w = ct_cdb_a0->write_begin(var_obj);
            ct_cdb_a0->set_uint64(var_w, MATERIAL_VAR_HANDLER_PROP,
                                  handler.idx);
            ct_cdb_a0->write_commit(var_w);
        }
    }
}

static void offline(uint64_t name,
                    uint64_t obj) {
    CT_UNUSED(name, obj);
}

static uint64_t cdb_type() {
    return MATERIAL_TYPE;
}

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
    const char *name = ct_cdb_a0->read_str(variable, MATERIAL_VAR_NAME_PROP,
                                           "");


    float size[2];
    ct_debugui_a0->GetWindowSize(size);
    size[1] = size[0] = 64;

    uint64_t var_value = ct_cdb_a0->read_uint64(variable,
                                                MATERIAL_VAR_VALUE_PROP, 0);

    ct_render_texture_handle_t texture;
    texture = ct_texture_a0->get(var_value);

    ct_debugui_a0->Image(texture,
                         size,
                         (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                         (float[4]) {0.0f, 0.0f, 0.0, 0.0f});

    ct_debugui_a0->SameLine(0, 0);

    ct_editor_ui_a0->ui_resource(variable, MATERIAL_VAR_VALUE_PROP, name,
                                 TEXTURE_TYPE, variable);

//    ct_debugui_a0->InputText(str, buff, strlen(buff),
//                             DebugInputTextFlags_ReadOnly, 0, NULL);
//



}

static void draw_property(uint64_t material) {
    uint64_t layers_obj = ct_cdb_a0->read_ref(material, MATERIAL_LAYERS, 0);

    uint64_t layer_count = ct_cdb_a0->prop_count(layers_obj);
    uint64_t layer_keys[layer_count];
    ct_cdb_a0->prop_keys(layers_obj, layer_keys);

    for (int i = 0; i < layer_count; ++i) {
        char label[128];

        uint64_t layer;
        layer = ct_cdb_a0->read_ref(layers_obj, layer_keys[i], 0);

        const char *layer_name = ct_cdb_a0->read_str(layer,
                                                     MATERIAL_LAYER_NAME, NULL);

        snprintf(label, CT_ARRAY_LEN(label), "Layer: %s", layer_name);
        if (ct_debugui_a0->TreeNodeEx(label,
                                      DebugUITreeNodeFlags_DefaultOpen)) {

            ct_editor_ui_a0->ui_str(layer, MATERIAL_LAYER_NAME, "Layer name",
                                    i);

            ct_editor_ui_a0->ui_resource(layer, MATERIAL_SHADER_PROP, "Shader",
                                         SHADER_TYPE, i);

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

//                    case MAT_VAR_TEXTURE_HANDLER:
////                        ui_texture(var);
////                        break;

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

            ct_debugui_a0->TreePop();
        }
    }
}


static const char *display_name() {
    return "Material";
}

static struct ct_asset_property_i0 ct_asset_property_i0 = {
        .display_name = display_name,
        .draw = draw_property,
};

static struct ct_entity load(struct ct_resource_id resourceid,
                             struct ct_world world) {

    struct ct_entity ent = ct_ecs_a0->entity->spawn(world,
                                                    ct_hashlib_a0->id64(
                                                            "core/cube"));

    uint64_t obj = ent.h;

    uint64_t components = ct_cdb_a0->read_subobject(obj, ENTITY_COMPONENTS, 0);
    uint64_t mesh_c = ct_cdb_a0->read_subobject(components,
                                                MESH_RENDERER_COMPONENT, 0);

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(mesh_c);
    ct_cdb_a0->set_uint64(w, PROP_MATERIAL_ID, resourceid.name);
    ct_cdb_a0->write_commit(w);

    return ent;
}

static void unload(struct ct_resource_id resourceid,
                   struct ct_world world,
                   struct ct_entity entity) {
    ct_ecs_a0->entity->destroy(world, &entity, 1);
}

static struct ct_asset_preview_i0 ct_asset_preview_i0 = {
        .load = load,
        .unload = unload,
};

static void *get_interface(uint64_t name_hash) {
    if (name_hash == ASSET_PROPERTY) {
        return &ct_asset_property_i0;
    }

    if (name_hash == ASSET_PREVIEW) {
        return &ct_asset_preview_i0;
    }
    return NULL;
}

void material_compiler(const char *filename,
                       char **output_blob);

static struct ct_resource_i0 ct_resource_i0 = {
        .cdb_type = cdb_type,
        .online = online,
        .offline = offline,
        .compilator = material_compiler,
        .get_interface = get_interface
};


//==============================================================================
// Interface
//==============================================================================

static uint64_t create(uint64_t name) {
    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = MATERIAL_TYPE,
            .name = name,
    };

    uint64_t object = ct_resource_a0->get(rid);
    return ct_cdb_a0->create_from(ct_cdb_a0->db(), object);
}

static void set_texture_handler(uint64_t material,
                                uint64_t layer,
                                const char *slot,
                                struct ct_render_texture_handle texture) {
    uint64_t layers_obj = ct_cdb_a0->read_ref(material, MATERIAL_LAYERS, 0);
    uint64_t layer_obj = ct_cdb_a0->read_ref(layers_obj, layer, 0);
    uint64_t variables = ct_cdb_a0->read_ref(layer_obj,
                                             MATERIAL_VARIABLES_PROP,
                                             0);
    uint64_t var = ct_cdb_a0->read_ref(variables,
                                       ct_hashlib_a0->id64(slot), 0);
    ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(var);
    ct_cdb_a0->set_uint64(writer, MATERIAL_VAR_VALUE_PROP, texture.idx);
    ct_cdb_a0->set_uint64(writer, MATERIAL_VAR_TYPE_PROP,
                          MAT_VAR_TEXTURE_HANDLER);
    ct_cdb_a0->write_commit(writer);
}

static void submit(uint64_t material,
                   uint64_t _layer,
                   uint8_t viewid) {
    uint64_t layers_obj = ct_cdb_a0->read_ref(material, MATERIAL_LAYERS, 0);
    uint64_t layer = ct_cdb_a0->read_ref(layers_obj, _layer, 0);

    if (!layer) {
        return;
    }

    uint64_t variables = ct_cdb_a0->read_ref(layer,
                                             MATERIAL_VARIABLES_PROP,
                                             0);

    uint64_t key_count = ct_cdb_a0->prop_count(variables);
    uint64_t keys[key_count];
    ct_cdb_a0->prop_keys(variables, keys);

    uint8_t texture_stage = 0;

    for (int j = 0; j < key_count; ++j) {
        uint64_t var = ct_cdb_a0->read_ref(variables, keys[j], 0);
        uint64_t type = ct_cdb_a0->read_uint64(var, MATERIAL_VAR_TYPE_PROP, 0);

        ct_render_uniform_handle_t handle = {
                .idx = (uint16_t) ct_cdb_a0->read_uint64(var,
                                                         MATERIAL_VAR_HANDLER_PROP,
                                                         0)
        };

        switch (type) {
            case MAT_VAR_NONE:
                break;

            case MAT_VAR_INT: {
                uint32_t v = ct_cdb_a0->read_uint64(var,
                                                    MATERIAL_VAR_VALUE_PROP, 0);
                ct_renderer_a0->set_uniform(handle, &v, 1);
            }
                break;

            case MAT_VAR_TEXTURE: {
                uint64_t t = ct_cdb_a0->read_uint64(var,
                                                    MATERIAL_VAR_VALUE_PROP, 0);
                ct_render_texture_handle_t texture = ct_texture_a0->get(t);
                ct_renderer_a0->set_texture(texture_stage++, handle,
                                            texture, 0);
            }
                break;

            case MAT_VAR_TEXTURE_HANDLER: {
                uint64_t t = ct_cdb_a0->read_uint64(var,
                                                    MATERIAL_VAR_VALUE_PROP, 0);
                ct_renderer_a0->set_texture(texture_stage++, handle,
                                            (ct_render_texture_handle_t) {.idx=(uint16_t) t},
                                            0);
            }
                break;

            case MAT_VAR_COLOR4:
            case MAT_VAR_VEC4: {
                float v[4] = {1.0f, 1.0f, 1.0f, 1.0f};
                ct_cdb_a0->read_vec4(var, MATERIAL_VAR_VALUE_PROP, v),
                        ct_renderer_a0->set_uniform(handle, &v, 1);
            }
                break;

            case MAT_VAR_MAT44:
                break;
        }
    }

    uint64_t shader_obj = ct_resource_a0->get(
            (struct ct_resource_id) {
                    .name = ct_cdb_a0->read_uint64(layer,
                                                   MATERIAL_SHADER_PROP,
                                                   0),
                    .type = SHADER_TYPE,
            });

    if (!shader_obj) {
        return;
    }

    ct_render_program_handle_t shader = ct_shader_a0->get(shader_obj);

    uint64_t state = ct_cdb_a0->read_uint64(layer, MATERIAL_STATE_PROP, 0);

    ct_renderer_a0->set_state(state, 0);
    ct_renderer_a0->submit(viewid, shader, 0, false);
}

static struct ct_material_a0 material_api = {
        .create = create,
        .set_texture_handler = set_texture_handler,
        .submit = submit
};

struct ct_material_a0 *ct_material_a0 = &material_api;

static int init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0->system,
            .db = ct_cdb_a0->db()
    };
    api->register_api("ct_material_a0", &material_api);

    ct_api_a0->register_api(RESOURCE_I_NAME, &ct_resource_i0);

    materialcompiler_init(api);

    return 1;
}

static void shutdown() {
    ct_cdb_a0->destroy_db(_G.db);
}

CETECH_MODULE_DEF(
        material,
        {
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_resource_a0);
            CT_INIT_API(api, ct_os_a0);
            CT_INIT_API(api, ct_hashlib_a0);
            CT_INIT_API(api, ct_texture_a0);
            CT_INIT_API(api, ct_shader_a0);
            CT_INIT_API(api, ct_cdb_a0);
            CT_INIT_API(api, ct_renderer_a0);
        },
        {
            CT_UNUSED(reload);
            init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            shutdown();
        }
)