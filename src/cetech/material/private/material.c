//==============================================================================
// Include
//==============================================================================
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "celib/memory/allocator.h"
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include <celib/module.h>

#include <celib/macros.h>
#include <celib/log.h>

#include "cetech/resource/resource.h"
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/material/material.h>
#include <cetech/shader/shader.h>
#include <cetech/texture/texture.h>
#include <cetech/debugui/debugui.h>
#include <cetech/ecs/ecs.h>
#include <cetech/mesh/mesh_renderer.h>
#include <cetech/editor/resource_preview.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/resource/resourcedb.h>
#include <cetech/editor/property.h>

#include <bgfx/defines.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/transform/transform.h>
#include <cetech/mesh/primitive_mesh.h>


//==============================================================================
// Defines
//==============================================================================

#define _G MaterialGlobals
#define LOG_WHERE "material"


//==============================================================================
// GLobals
//==============================================================================

static struct _G {
    struct ce_cdb_t0 db;
    struct ce_alloc_t0 *allocator;
} _G;


//==============================================================================
// Resource
//==============================================================================

enum material_variable_type {
    MAT_VAR_NONE = 0,
    MAT_VAR_INT,
    MAT_VAR_TEXTURE,
    MAT_VAR_TEXTURE_HANDLER, //TODO: RENAME
    MAT_VAR_COLOR4,
    MAT_VAR_VEC4,
};


static bgfx_uniform_type_t _type_to_bgfx[] = {
        [MAT_VAR_NONE] = BGFX_UNIFORM_TYPE_COUNT,
        [MAT_VAR_INT] = BGFX_UNIFORM_TYPE_SAMPLER,
        [MAT_VAR_TEXTURE] = BGFX_UNIFORM_TYPE_SAMPLER,
        [MAT_VAR_TEXTURE_HANDLER] = BGFX_UNIFORM_TYPE_SAMPLER,
        [MAT_VAR_VEC4] = BGFX_UNIFORM_TYPE_VEC4,
};

static uint64_t _str_to_type(const char *type) {
    if (!strcmp(type, "texture")) {
        return MAT_VAR_TEXTURE;
    } else if (!strcmp(type, "texture_handler")) {
        return MAT_VAR_TEXTURE_HANDLER;
    } else if ((!strcmp(type, "color")) || (!strcmp(type, "vec4"))) {
        return (!strcmp(type, "color")) ? MAT_VAR_COLOR4 : MAT_VAR_VEC4;
    }
    return MAT_VAR_NONE;
}

static void online(uint64_t name,
                   uint64_t obj) {
    CE_UNUSED(name);

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    uint64_t layers_obj = ce_cdb_a0->read_subobject(reader, MATERIAL_LAYERS, 0);
    const ce_cdb_obj_o0 *layers_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                         layers_obj);

    const uint64_t layers_n = ce_cdb_a0->prop_count(layers_reader);
    const uint64_t *layers_keys = ce_cdb_a0->prop_keys(layers_reader);

    for (int i = 0; i < layers_n; ++i) {
        uint64_t layer_obj = ce_cdb_a0->read_subobject(layers_reader,
                                                       layers_keys[i], 0);


        const ce_cdb_obj_o0 *layer_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                            layer_obj);

        uint64_t variables_obj = ce_cdb_a0->read_subobject(layer_reader,
                                                           MATERIAL_VARIABLES_PROP,
                                                           0);

        const ce_cdb_obj_o0 *vars_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                           variables_obj);

        const uint64_t variables_n = ce_cdb_a0->prop_count(vars_reader);
        const uint64_t *variables_keys = ce_cdb_a0->prop_keys(vars_reader);

        for (int k = 0; k < variables_n; ++k) {
//            if (ce_cdb_a0->prop_type(vars_reader, variables_keys[k])) {
//                continue;
//            }

            uint64_t var_obj = ce_cdb_a0->read_subobject(vars_reader,
                                                         variables_keys[k], 0);

            const ce_cdb_obj_o0 *var_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                              var_obj);

            const char *uniform_name = ce_cdb_a0->read_str(var_reader,
                                                           MATERIAL_VAR_NAME_PROP,
                                                           0);

            uint64_t type = _str_to_type(ce_cdb_a0->read_str(var_reader,
                                                             MATERIAL_VAR_TYPE_PROP,
                                                             ""));

            if (!type) {
                continue;
            }

            const bgfx_uniform_handle_t handler = \
            ct_gfx_a0->bgfx_create_uniform(uniform_name, _type_to_bgfx[type],
                                           1);

            ce_cdb_obj_o0 *var_w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                          var_obj);
            ce_cdb_a0->set_uint64(var_w, MATERIAL_VAR_HANDLER_PROP,
                                  handler.idx);
            ce_cdb_a0->write_commit(var_w);
        }
    }
}

static void offline(uint64_t name,
                    uint64_t obj) {
    CE_UNUSED(name, obj);
}

static uint64_t cdb_type() {
    return MATERIAL_TYPE;
}

static void ui_vec4(uint64_t var) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), var);

    const char *str = ce_cdb_a0->read_str(reader, MATERIAL_VAR_NAME_PROP, "");
    ct_editor_ui_a0->prop_vec4(var,
                               (uint64_t[]) {MATERIAL_VAR_VALUE_PROP_X,
                                             MATERIAL_VAR_VALUE_PROP_Y,
                                             MATERIAL_VAR_VALUE_PROP_Z,
                                             MATERIAL_VAR_VALUE_PROP_W},
                               str, (ui_vec4_p0) {});
}

static void ui_color4(uint64_t var) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), var);
    const char *str = ce_cdb_a0->read_str(reader, MATERIAL_VAR_NAME_PROP, "");
    ct_editor_ui_a0->prop_vec4(var,
                               (uint64_t[]) {MATERIAL_VAR_VALUE_PROP_X,
                                             MATERIAL_VAR_VALUE_PROP_Y,
                                             MATERIAL_VAR_VALUE_PROP_Z,
                                             MATERIAL_VAR_VALUE_PROP_W}, str,
                               (ui_vec4_p0) {.color=true});
}

static void ui_texture(uint64_t var,
                       uint64_t context) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), var);

    const char *name = ce_cdb_a0->read_str(reader, MATERIAL_VAR_NAME_PROP,
                                           "");

    ct_editor_ui_a0->prop_resource(var, MATERIAL_VAR_VALUE_PROP, name,
                                   TEXTURE_TYPE, context, var);
}

static void draw_property(uint64_t material,
                          uint64_t context) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), material);

    uint64_t layers_obj = ce_cdb_a0->read_ref(reader, MATERIAL_LAYERS, 0);

    if (!layers_obj) {
        return;
    }

    const ce_cdb_obj_o0 *layers_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                         layers_obj);

    uint64_t layer_count = ce_cdb_a0->prop_count(layers_reader);
    const uint64_t *layer_keys = ce_cdb_a0->prop_keys(layers_reader);
    for (int i = 0; i < layer_count; ++i) {
        if (CDB_INSTANCE_PROP == layer_keys[i]) {
            continue;
        }

        uint64_t layer;
        layer = ce_cdb_a0->read_subobject(layers_reader, layer_keys[i], 0);

        const ce_cdb_obj_o0 *layer_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                            layer);

        const char *layer_name = ce_cdb_a0->read_str(layer_reader,
                                                     MATERIAL_LAYER_NAME, NULL);
        char layer_label[128] = {};
        snprintf(layer_label, CE_ARRAY_LEN(layer_label), "Layer %s##layer_%llx",
                 layer_name, layer);

        bool open = ct_debugui_a0->TreeNodeEx(layer_label,
                                              DebugUITreeNodeFlags_DefaultOpen);
        if (open) {
            ct_editor_ui_a0->prop_str(layer, MATERIAL_LAYER_NAME,
                                      "Layer name", i);

            ct_editor_ui_a0->prop_resource(layer, MATERIAL_SHADER_PROP,
                                           "Shader",
                                           SHADER_TYPE, context, i);

            uint64_t variables;
            variables = ce_cdb_a0->read_ref(layer_reader,
                                            MATERIAL_VARIABLES_PROP, 0);

            const ce_cdb_obj_o0 *vars_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                               variables);

            uint64_t count = ce_cdb_a0->prop_count(vars_reader);
            const uint64_t *keys = ce_cdb_a0->prop_keys(vars_reader);

            for (int j = 0; j < count; ++j) {
                uint64_t var;
                var = ce_cdb_a0->read_subobject(vars_reader, keys[j], 0);

                if (!var) {
                    continue;
                }

                const ce_cdb_obj_o0 *var_reader = ce_cdb_a0->read(
                        ce_cdb_a0->db(), var);

                const char *type = ce_cdb_a0->read_str(var_reader,
                                                       MATERIAL_VAR_TYPE_PROP,
                                                       0);
                if (!type) continue;

                if (!strcmp(type, "texture")) {
                    ui_texture(var, context);
                } else if (!strcmp(type, "vec4")) {
                    ui_vec4(var);
                } else if (!strcmp(type, "color")) {
                    ui_color4(var);
                } else if (!strcmp(type, "mat4")) {
                }
            }

            ct_debugui_a0->TreePop();
        }
    }
}

static struct ct_entity_t0 load(uint64_t resource,
                                ct_world_t0 world) {
    struct ct_entity_t0 ent = {};
    ct_ecs_a0->create(world, &ent, 1);

    ct_ecs_a0->add(
            world, ent,
            (uint64_t[]) {
                    TRANSFORM_COMPONENT,
                    PRIMITIVE_MESH_COMPONENT,
            }, 2,
            (void *[]) {
                    &(ct_transform_comp) {
                            .t.pos = {0.0f, 0.0f, 13.0f},
                            .t.scl = CE_TRANFORM_INIT.scl,
                            .t.rot  = CE_TRANFORM_INIT.rot
                    },

                    &(ct_primitive_mesh) {
                            .material = resource
                    },
            }
    );

    return ent;
}

static struct ct_resource_preview_i0 ct_resource_preview_api = {
        .load = load,
};

static void *get_interface(uint64_t name_hash) {
    if (name_hash == RESOURCE_PREVIEW_I) {
        return &ct_resource_preview_api;
    }
    return NULL;
}

static const char *display_icon() {
    return ICON_FA_FILE_IMAGE_O;
}


static struct ct_resource_i0 ct_resource_api = {
        .cdb_type = cdb_type,
        .display_icon = display_icon,
        .online = online,
        .offline = offline,
        .get_interface = get_interface
};


//==============================================================================
// Interface
//==============================================================================

static uint64_t create(uint64_t name) {
    struct ct_resource_id_t0 rid = (ct_resource_id_t0) {
            .uid = name,
    };

    uint64_t object = rid.uid;
    return object;
}

static uint64_t _find_slot_by_name(uint64_t variables,
                                   const char *name) {
    const ce_cdb_obj_o0 *vars_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                       variables);

    uint64_t k_n = ce_cdb_a0->prop_count(vars_reader);
    const uint64_t *k = ce_cdb_a0->prop_keys(vars_reader);
    for (int i = 0; i < k_n; ++i) {
        uint64_t key = k[i];

        uint64_t var = ce_cdb_a0->read_subobject(vars_reader, key, 0);

        const ce_cdb_obj_o0 *var_reader = ce_cdb_a0->read(ce_cdb_a0->db(), var);
        const char *var_name = ce_cdb_a0->read_str(var_reader,
                                                   MATERIAL_VAR_NAME_PROP,
                                                   "");

        if (!strcmp(name, var_name)) {
            return var;
        }

    }

    return 0;
}

static void set_texture_handler(uint64_t material,
                                uint64_t layer,
                                const char *slot,
                                bgfx_texture_handle_t texture) {
    const ce_cdb_obj_o0 *mat_reader = ce_cdb_a0->read(ce_cdb_a0->db(), material);

    uint64_t layers_obj = ce_cdb_a0->read_ref(mat_reader, MATERIAL_LAYERS, 0);
    const ce_cdb_obj_o0 *layers_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                         layers_obj);

    uint64_t layer_obj = ce_cdb_a0->read_ref(layers_reader, layer, 0);
    const ce_cdb_obj_o0 *layer_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                        layer_obj);

    uint64_t variables = ce_cdb_a0->read_ref(layer_reader,
                                             MATERIAL_VARIABLES_PROP,
                                             0);

    uint64_t var = _find_slot_by_name(variables, slot);
    if (!var) {
//        uint64_t name = ce_cdb_a0->read_uint64(mat_reader, RESOURCE_NAME_PROP, 0);

        ce_log_a0->warning(LOG_WHERE, "invalid slot: %s", slot);
        return;
    }

    ce_cdb_obj_o0 *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), var);
    ce_cdb_a0->set_uint64(writer, MATERIAL_VAR_VALUE_PROP, texture.idx);
    ce_cdb_a0->set_str(writer, MATERIAL_VAR_TYPE_PROP, "texture_handler");
    ce_cdb_a0->write_commit(writer);
}

static struct {
    uint64_t name;
    uint64_t e;
} _tbl[] = {
        {.name = 0, .e = 0},
        {.name = RENDER_STATE_RGB_WRITE, .e = BGFX_STATE_WRITE_RGB},
        {.name = RENDER_STATE_ALPHA_WRITE, .e = BGFX_STATE_WRITE_A},
        {.name = RENDER_STATE_DEPTH_WRITE, .e = BGFX_STATE_WRITE_Z},
        {.name = RENDER_STATE_DEPTH_TEST_LESS, .e = BGFX_STATE_DEPTH_TEST_LESS},
        {.name = RENDER_STATE_CULL_CCW, .e = BGFX_STATE_CULL_CCW},
        {.name = RENDER_STATE_CULL_CW, .e = BGFX_STATE_CULL_CW},
        {.name = RENDER_STATE_MSAA, .e = BGFX_STATE_MSAA},
};

uint64_t render_state_to_enum(uint64_t name) {

    for (uint32_t i = 1; i < CE_ARRAY_LEN(_tbl); ++i) {
        if (_tbl[i].name != name) {
            continue;
        }

        return _tbl[i].e;
    }

    return 0;
}


uint64_t _get_render_state(uint64_t layer) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), layer);
    uint64_t render_state = ce_cdb_a0->read_subobject(reader,
                                                      ce_id_a0->id64(
                                                              "render_state"),
                                                      0);

    if (render_state) {
        uint64_t curent_render_state = 0;

        const ce_cdb_obj_o0 *rs_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                         render_state);

        const uint64_t render_state_count = ce_cdb_a0->prop_count(rs_reader);
        const uint64_t *render_state_keys = ce_cdb_a0->prop_keys(rs_reader);

        for (uint32_t i = 0; i < render_state_count; ++i) {
            curent_render_state |= render_state_to_enum(render_state_keys[i]);
        }

        return curent_render_state;
    }

    return 0;
}


static void submit(uint64_t material,
                   uint64_t _layer,
                   uint8_t viewid) {
    const ce_cdb_obj_o0 *mat_reader = ce_cdb_a0->read(ce_cdb_a0->db(), material);

    uint64_t layers_obj = ce_cdb_a0->read_ref(mat_reader, MATERIAL_LAYERS, 0);
    if (!layers_obj) {
        return;
    }

    const ce_cdb_obj_o0 *layers_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                         layers_obj);

    uint64_t layer = ce_cdb_a0->read_ref(layers_reader, _layer, 0);

    if (!layer) {
        return;
    }

    const ce_cdb_obj_o0 *layer_reader = ce_cdb_a0->read(ce_cdb_a0->db(), layer);

    uint64_t variables = ce_cdb_a0->read_ref(layer_reader,
                                             MATERIAL_VARIABLES_PROP,
                                             0);

    const ce_cdb_obj_o0 *vars_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                       variables);

    uint64_t key_count = ce_cdb_a0->prop_count(vars_reader);
    const uint64_t *keys = ce_cdb_a0->prop_keys(vars_reader);

    uint8_t texture_stage = 0;


    for (int j = 0; j < key_count; ++j) {
        uint64_t var = ce_cdb_a0->read_ref(vars_reader, keys[j], 0);
        const ce_cdb_obj_o0 *var_reader = ce_cdb_a0->read(ce_cdb_a0->db(), var);

        uint64_t type = _str_to_type(
                ce_cdb_a0->read_str(var_reader, MATERIAL_VAR_TYPE_PROP, ""));

        bgfx_uniform_handle_t handle = {
                .idx = (uint16_t) ce_cdb_a0->read_uint64(var_reader,
                                                         MATERIAL_VAR_HANDLER_PROP,
                                                         0)
        };

        switch (type) {
            case MAT_VAR_NONE:
                break;

            case MAT_VAR_INT: {
                uint64_t v = ce_cdb_a0->read_uint64(var_reader,
                                                    MATERIAL_VAR_VALUE_PROP, 0);
                ct_gfx_a0->bgfx_set_uniform(handle, &v, 1);
            }
                break;

            case MAT_VAR_TEXTURE: {
                uint64_t tn = ce_cdb_a0->read_uint64(var_reader,
                                                     MATERIAL_VAR_VALUE_PROP,
                                                     0);

                ct_gfx_a0->bgfx_set_texture(texture_stage++, handle,
                                            ct_texture_a0->get(tn), 0);
            }
                break;

            case MAT_VAR_TEXTURE_HANDLER: {
                uint64_t t = ce_cdb_a0->read_uint64(var_reader,
                                                    MATERIAL_VAR_VALUE_PROP, 0);
                ct_gfx_a0->bgfx_set_texture(texture_stage++, handle,
                                            (bgfx_texture_handle_t) {.idx=(uint16_t) t},
                                            0);
            }
                break;

            case MAT_VAR_COLOR4:
            case MAT_VAR_VEC4: {
                float v[4] = {
                        ce_cdb_a0->read_float(var_reader,
                                              MATERIAL_VAR_VALUE_PROP_X, 1.0f),
                        ce_cdb_a0->read_float(var_reader,
                                              MATERIAL_VAR_VALUE_PROP_Y, 1.0f),
                        ce_cdb_a0->read_float(var_reader,
                                              MATERIAL_VAR_VALUE_PROP_Z, 1.0f),
                        ce_cdb_a0->read_float(var_reader,
                                              MATERIAL_VAR_VALUE_PROP_W, 1.0f)
                };

                ct_gfx_a0->bgfx_set_uniform(handle, &v, 1);
            }
                break;

        }
    }

    uint64_t shader = ce_cdb_a0->read_ref(layer_reader,
                                          MATERIAL_SHADER_PROP,
                                          0);

    uint64_t shader_obj = shader;

    if (!shader_obj) {
        return;
    }

    bgfx_program_handle_t shaderp = ct_shader_a0->get(shader_obj);

    uint64_t state = _get_render_state(layer);

    ct_gfx_a0->bgfx_set_state(state, 0);
    ct_gfx_a0->bgfx_submit(viewid, shaderp, 0, false);
}

static struct ct_material_a0 material_api = {
        .create = create,
        .set_texture_handler = set_texture_handler,
        .submit = submit
};

struct ct_material_a0 *ct_material_a0 = &material_api;

static struct ct_property_editor_i0 _property_editor_api = {
        .cdb_type = cdb_type,
        .draw_ui = draw_property,
};

static int init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .db = ce_cdb_a0->db()
    };

    api->register_api(CT_MATERIAL_API, &material_api, sizeof(material_api));
    api->register_api(RESOURCE_I, &ct_resource_api, sizeof(ct_resource_api));
    api->register_api(PROPERTY_EDITOR_INTERFACE, &_property_editor_api,
                      sizeof(_property_editor_api));

    return 1;
}

static void shutdown() {
}

void CE_MODULE_LOAD(material)(struct ce_api_a0 *api,
                              int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_texture_a0);
    CE_INIT_API(api, ct_shader_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_renderer_a0);
    init(api);
}

void CE_MODULE_UNLOAD(material)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    shutdown();
}
