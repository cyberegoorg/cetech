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
#include <cetech/mesh/static_mesh.h>
#include <cetech/resource/resource_preview.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/resource/resourcedb.h>
#include <cetech/property_editor/property_editor.h>

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
    ce_cdb_t0 db;
    ce_alloc_t0 *allocator;
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

static uint64_t _cdb_type_to_type(uint64_t type) {
    if (type == MATERIAL_VAR_TYPE_TEXTURE) {
        return MAT_VAR_TEXTURE;

    } else if (type == MATERIAL_VAR_TYPE_TEXTURE_HANDLER) {
        return MAT_VAR_TEXTURE_HANDLER;

    } else if (type == MATERIAL_VAR_TYPE_COLOR) {
        return MAT_VAR_COLOR4;

    } else if (type == MATERIAL_VAR_TYPE_VEC4) {
        return MAT_VAR_VEC4;
    }
    return MAT_VAR_NONE;
}

static void online(ce_cdb_t0 db,
                   uint64_t obj) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, obj);

    uint64_t layers_n = ce_cdb_a0->read_objset_num(reader, MATERIAL_LAYERS);
    uint64_t layers_keys[layers_n];
    ce_cdb_a0->read_objset(reader, MATERIAL_LAYERS, layers_keys);

    for (int i = 0; i < layers_n; ++i) {
        uint64_t layer_obj = layers_keys[i];

        const ce_cdb_obj_o0 *layer_reader = ce_cdb_a0->read(db, layer_obj);

        uint64_t variables_n = ce_cdb_a0->read_objset_num(layer_reader, MATERIAL_VARIABLES_PROP);
        uint64_t variables_keys[variables_n];
        ce_cdb_a0->read_objset(layer_reader, MATERIAL_VARIABLES_PROP, variables_keys);

        for (int k = 0; k < variables_n; ++k) {
            uint64_t var_obj = variables_keys[k];

            const ce_cdb_obj_o0 *var_reader = ce_cdb_a0->read(db,
                                                              var_obj);

            const char *uniform_name = ce_cdb_a0->read_str(var_reader,
                                                           MATERIAL_VAR_NAME_PROP,
                                                           0);

            uint64_t var_type = ce_cdb_a0->obj_type(db, var_obj);

            uint64_t type = _cdb_type_to_type(var_type);
            if (!type) {
                continue;
            }

            const bgfx_uniform_handle_t handler = ct_gfx_a0->bgfx_create_uniform(uniform_name,
                                                                                 _type_to_bgfx[type],
                                                                                 1);

            ce_cdb_obj_o0 *var_w = ce_cdb_a0->write_begin(db, var_obj);
            ce_cdb_a0->set_uint64(var_w, MATERIAL_VAR_HANDLER_PROP, handler.idx);
            ce_cdb_a0->write_commit(var_w);
        }
    }
}

static void offline(ce_cdb_t0 db,
                    uint64_t obj) {
    CE_UNUSED(db, obj);
}

static uint64_t cdb_type() {
    return MATERIAL_TYPE;
}

static void ui_vec4(uint64_t var) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), var);

    const char *str = ce_cdb_a0->read_str(reader, MATERIAL_VAR_NAME_PROP, "");
    ct_editor_ui_a0->prop_vec4(var,
                               str,
                               (uint64_t[]) {MATERIAL_VAR_VALUE_PROP_X,
                                             MATERIAL_VAR_VALUE_PROP_Y,
                                             MATERIAL_VAR_VALUE_PROP_Z,
                                             MATERIAL_VAR_VALUE_PROP_W},
                               (ui_vec4_p0) {});
}

static void ui_color4(uint64_t var) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), var);
    const char *str = ce_cdb_a0->read_str(reader, MATERIAL_VAR_NAME_PROP, "");
    ct_editor_ui_a0->prop_vec4(var, str,
                               (uint64_t[]) {MATERIAL_VAR_VALUE_PROP_X,
                                             MATERIAL_VAR_VALUE_PROP_Y,
                                             MATERIAL_VAR_VALUE_PROP_Z,
                                             MATERIAL_VAR_VALUE_PROP_W},
                               (ui_vec4_p0) {.color=true});
}

static void ui_texture(uint64_t var,
                       uint64_t context) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), var);

    const char *name = ce_cdb_a0->read_str(reader, MATERIAL_VAR_NAME_PROP,
                                           "");

    ct_editor_ui_a0->prop_resource(var, name, MATERIAL_VAR_VALUE_PROP,
                                   TEXTURE_TYPE, context, var);
}

void draw_property(uint64_t material,
                   uint64_t context) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), material);

    uint64_t layer_count = ce_cdb_a0->read_objset_num(reader, MATERIAL_LAYERS);
    uint64_t layer_keys[layer_count];
    ce_cdb_a0->read_objset(reader, MATERIAL_LAYERS, layer_keys);

    for (int i = 0; i < layer_count; ++i) {
        uint64_t layer = layer_keys[i];
        const ce_cdb_obj_o0 *layer_reader = ce_cdb_a0->read(ce_cdb_a0->db(), layer);

        const char *layer_name = ce_cdb_a0->read_str(layer_reader,
                                                     MATERIAL_LAYER_NAME, NULL);
        char layer_label[128] = {};
        snprintf(layer_label, CE_ARRAY_LEN(layer_label), "Layer %s##layer_%llx",
                 layer_name, layer);

        bool open = ct_debugui_a0->TreeNodeEx(layer_label,
                                              DebugUITreeNodeFlags_DefaultOpen);
        if (open) {
            ct_editor_ui_a0->prop_str(layer, "Layer name", MATERIAL_LAYER_NAME, i);

            ct_editor_ui_a0->prop_resource(layer, "Shader", MATERIAL_SHADER_PROP,
                                           SHADER_TYPE, context, i);

            uint64_t count = ce_cdb_a0->read_objset_num(layer_reader, MATERIAL_VARIABLES_PROP);
            uint64_t keys[count];
            ce_cdb_a0->read_objset(layer_reader, MATERIAL_VARIABLES_PROP, keys);

            for (int j = 0; j < count; ++j) {
                uint64_t var = keys[j];

                if (!var) {
                    continue;
                }
                uint64_t cdb_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), var);
                uint64_t type = _cdb_type_to_type(cdb_type);

                if (!type) {
                    continue;
                }

                if (type == MAT_VAR_TEXTURE) {
                    ui_texture(var, context);
                } else if (type == MAT_VAR_VEC4) {
                    ui_vec4(var);
                } else if (MAT_VAR_COLOR4) {
                    ui_color4(var);
                }
            }

            ct_debugui_a0->TreePop();
        }
    }
}

static struct ct_entity_t0 load(uint64_t resource,
                                ct_world_t0 world) {
    ct_entity_t0 ent = {};
    ct_ecs_a0->create(world, &ent, 1);

    ct_ecs_a0->add(world, ent,
                   (ct_component_pair_t0[]) {
                           {
                                   .type = TRANSFORM_COMPONENT,
                                   .data = &(ct_transform_comp) {
                                           .scl = CE_VEC3_UNIT,
                                           .pos.z = 3,
                                   },
                           },
                           {
                                   .type = PRIMITIVE_MESH_COMPONENT,
                                   .data = &(ct_primitive_mesh) {
                                           .material = resource,
                                   },
                           }
                   }, 2);

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


static const char* name() {
    return "material";
}


static struct ct_resource_i0 ct_resource_api = {
        .cdb_type = cdb_type,
        .name = name,
        .display_icon = display_icon,
        .online = online,
        .offline = offline,
        .get_interface = get_interface
};


//==============================================================================
// Interface
//==============================================================================

static uint64_t create(uint64_t name) {
    ct_resource_id_t0 rid = (ct_resource_id_t0) {
            .uid = name,
    };

    uint64_t object = rid.uid;
    return object;
}

static uint64_t _find_slot_by_name(uint64_t layer,
                                   const char *name) {

    const ce_cdb_obj_o0 *layer_reader = ce_cdb_a0->read(ce_cdb_a0->db(), layer);
    uint64_t k_n = ce_cdb_a0->read_objset_num(layer_reader, MATERIAL_VARIABLES_PROP);
    uint64_t k[k_n];
    ce_cdb_a0->read_objset(layer_reader, MATERIAL_VARIABLES_PROP, k);

    for (int i = 0; i < k_n; ++i) {
        uint64_t var = k[i];

        const ce_cdb_obj_o0 *var_reader = ce_cdb_a0->read(ce_cdb_a0->db(), var);
        const char *var_name = ce_cdb_a0->read_str(var_reader, MATERIAL_VAR_NAME_PROP, "");

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


    uint64_t layers_n = ce_cdb_a0->read_objset_num(mat_reader, MATERIAL_LAYERS);
    uint64_t layers_keys[layers_n];
    ce_cdb_a0->read_objset(mat_reader, MATERIAL_LAYERS, layers_keys);

    for (int i = 0; i < layers_n; ++i) {
        uint64_t layer_obj = layers_keys[i];

        const ce_cdb_obj_o0 *layer_reader = ce_cdb_a0->read(ce_cdb_a0->db(), layer_obj);
        const char *layer_name = ce_cdb_a0->read_str(layer_reader, MATERIAL_LAYER_NAME, 0);
        if (ce_id_a0->id64(layer_name) != layer) {
            continue;
        }

        uint64_t var = _find_slot_by_name(layer_obj, slot);
        if (!var) {
            ce_log_a0->warning(LOG_WHERE, "invalid slot: %s", slot);
            return;
        }

        uint64_t t = 0;
        uint64_t var_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), var);

        if (var_type != MATERIAL_VAR_TYPE_TEXTURE_HANDLER) {
            t = ce_cdb_a0->create_object(ce_cdb_a0->db(), MATERIAL_VAR_TYPE_TEXTURE_HANDLER);
            ce_cdb_a0->destroy_object(ce_cdb_a0->db(), t);
        } else {
            t = var;
        }

        ce_cdb_obj_o0 *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), t);
        ce_cdb_a0->set_uint64(writer, MATERIAL_VAR_VALUE_PROP, texture.idx);
        ce_cdb_a0->set_str(writer, MATERIAL_VAR_NAME_PROP, slot);
        ce_cdb_a0->write_commit(writer);

        writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), layer_obj);
        ce_cdb_a0->objset_add_obj(writer, MATERIAL_VARIABLES_PROP, t);
        ce_cdb_a0->write_commit(writer);
        break;
    }
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
    uint64_t render_state = ce_cdb_a0->read_subobject(reader, ce_id_a0->id64("render_state"), 0);

    if (render_state) {
        uint64_t curent_render_state = 0;

        const ce_cdb_obj_o0 *rs_reader = ce_cdb_a0->read(ce_cdb_a0->db(), render_state);
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

    uint64_t layers_n = ce_cdb_a0->read_objset_num(mat_reader, MATERIAL_LAYERS);
    uint64_t layers_keys[layers_n];
    ce_cdb_a0->read_objset(mat_reader, MATERIAL_LAYERS, layers_keys);

    for (int i = 0; i < layers_n; ++i) {
        uint64_t layer = layers_keys[i];

        const ce_cdb_obj_o0 *layer_reader = ce_cdb_a0->read(ce_cdb_a0->db(), layer);

        uint64_t key_count = ce_cdb_a0->read_objset_num(layer_reader, MATERIAL_VARIABLES_PROP);
        uint64_t keys[key_count];
        ce_cdb_a0->read_objset(layer_reader, MATERIAL_VARIABLES_PROP, keys);

        uint8_t texture_stage = 0;
        for (int j = 0; j < key_count; ++j) {
            uint64_t var = keys[j];
            const ce_cdb_obj_o0 *var_reader = ce_cdb_a0->read(ce_cdb_a0->db(), var);
            uint64_t var_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), var);
            uint64_t type = _cdb_type_to_type(var_type);

            bgfx_uniform_handle_t handle = {
                    .idx = (uint16_t) ce_cdb_a0->read_uint64(var_reader,
                                                             MATERIAL_VAR_HANDLER_PROP, 0)
            };

            switch (type) {
                case MAT_VAR_NONE:
                    break;

                case MAT_VAR_INT: {
                    uint64_t v = ce_cdb_a0->read_uint64(var_reader, MATERIAL_VAR_VALUE_PROP, 0);
                    ct_gfx_a0->bgfx_set_uniform(handle, &v, 1);
                }
                    break;

                case MAT_VAR_TEXTURE: {
                    uint64_t tn = ce_cdb_a0->read_uint64(var_reader, MATERIAL_VAR_VALUE_PROP, 0);
                    ct_gfx_a0->bgfx_set_texture(texture_stage++, handle, ct_texture_a0->get(tn), 0);
                }
                    break;

                case MAT_VAR_TEXTURE_HANDLER: {
                    uint64_t t = ce_cdb_a0->read_uint64(var_reader, MATERIAL_VAR_VALUE_PROP, 0);
                    ct_gfx_a0->bgfx_set_texture(texture_stage++, handle,
                                                (bgfx_texture_handle_t) {.idx=(uint16_t) t}, 0);
                }
                    break;

                case MAT_VAR_COLOR4:
                case MAT_VAR_VEC4: {
                    float v[4] = {
                            ce_cdb_a0->read_float(var_reader, MATERIAL_VAR_VALUE_PROP_X, 1.0f),
                            ce_cdb_a0->read_float(var_reader, MATERIAL_VAR_VALUE_PROP_Y, 1.0f),
                            ce_cdb_a0->read_float(var_reader, MATERIAL_VAR_VALUE_PROP_Z, 1.0f),
                            ce_cdb_a0->read_float(var_reader, MATERIAL_VAR_VALUE_PROP_W, 1.0f)
                    };

                    ct_gfx_a0->bgfx_set_uniform(handle, &v, 1);
                }
                    break;
                default:
                    CE_UNUSED(1)
                    break;

            }
        }

        uint64_t shader = ce_cdb_a0->read_ref(layer_reader, MATERIAL_SHADER_PROP, 0);

        uint64_t shader_obj = shader;

        if (!shader_obj) {
            return;
        }

        bgfx_program_handle_t shaderp = ct_shader_a0->get(shader_obj);

        uint64_t state = _get_render_state(layer);

        ct_gfx_a0->bgfx_set_state(state, 0);
        ct_gfx_a0->bgfx_submit(viewid, shaderp, 0, false);
    }
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

typedef struct ct_material_obj_t0 {
    uint64_t layers;
} ct_material_obj_t0;

typedef struct ct_material_layer_obj_t0 {
    const char *name;
    uint64_t shader;
    uint64_t render_state;
    uint64_t variables;
} ct_material_layer_obj_t0;

typedef struct ct_material_variable_tex_obj_t0 {
    const char *name;
    uint64_t value;
    uint64_t handler;
} ct_material_variable_tex_obj_t0;

typedef struct ct_material_variable_texh_obj_t0 {
    const char *name;
    uint64_t value;
    uint64_t handler;
} ct_material_variable_texh_obj_t0;

typedef struct ct_material_variable_vec4_obj_t0 {
    const char *name;
    uint64_t handler;
//    float x, y, z, w;
    ce_vec4_t value;
} ct_material_variable_vec4_obj_t0;


typedef struct ct_material_variable_color_obj_t0 {
    const char *name;
    uint64_t handler;
    ce_vec4_t value;
//    float x, y, z, w;
} ct_material_variable_color_obj_t0;


static const ce_cdb_prop_def_t0 material_prop[] = {
        {
                .name = "layers",
                .type = CE_CDB_TYPE_SET_SUBOBJECT,
        }
};


static const ce_cdb_prop_def_t0 material_layer_prop[] = {
        {
                .name = "name",
                .type = CE_CDB_TYPE_STR,
        },
        {
                .name = "shader",
                .type = CE_CDB_TYPE_REF,
                .obj_type=SHADER_TYPE,
        },
        {
                .name = "render_state",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = MATERIAL_RENDER_STATE
        },
        {
                .name = "variables",
                .type = CE_CDB_TYPE_SET_SUBOBJECT,
        },
};

static const ce_cdb_prop_def_t0 render_state_prop[] = {
        {
                .name = "rgb_write",
                .type = CE_CDB_TYPE_BOOL,
        },
        {
                .name = "alpha_write",
                .type = CE_CDB_TYPE_BOOL,
        },
        {
                .name = "depth_write",
                .type = CE_CDB_TYPE_BOOL,
        },
        {
                .name = "depth_test_less",
                .type = CE_CDB_TYPE_BOOL,
        },
        {
                .name = "cull_ccw",
                .type = CE_CDB_TYPE_BOOL,
        },
        {
                .name = "cull_cw",
                .type = CE_CDB_TYPE_BOOL,
        },
        {
                .name = "msaa",
                .type = CE_CDB_TYPE_BOOL,
        },
};

static const ce_cdb_prop_def_t0 material_texture_prop[] = {
        {
                .name = "name",
                .type = CE_CDB_TYPE_STR,
        },

        {
                .name = "value",
                .type = CE_CDB_TYPE_REF,
                .obj_type = TEXTURE_TYPE,
        },

        {
                .name = "handler",
                .type = CE_CDB_TYPE_UINT64,
        },
};

static const ce_cdb_prop_def_t0 material_texture_handler_prop[] = {
        {
                .name = "name",
                .type = CE_CDB_TYPE_STR,
        },

        {
                .name = "value",
                .type = CE_CDB_TYPE_UINT64,
        },

        {
                .name = "handler",
                .type = CE_CDB_TYPE_UINT64,
        },
};

static const ce_cdb_prop_def_t0 material_vec4_prop[] = {
        {
                .name = "name", .type = CE_CDB_TYPE_STR,
        },
        {.name = "handler",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "x",
                .type = CE_CDB_TYPE_FLOAT,
        },
        {
                .name = "y",
                .type = CE_CDB_TYPE_FLOAT,
        },
        {
                .name = "z",
                .type = CE_CDB_TYPE_FLOAT,
        },
        {
                .name = "w",
                .type = CE_CDB_TYPE_FLOAT,
        },
};

static const ce_cdb_prop_def_t0 material_color_prop[] = {
        {
                .name = "name", .type = CE_CDB_TYPE_STR,
        },
        {
                .name = "handler",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "x",
                .type = CE_CDB_TYPE_FLOAT,

        },
        {
                .name = "y",
                .type = CE_CDB_TYPE_FLOAT,
        },

        {
                .name = "z",
                .type = CE_CDB_TYPE_FLOAT,
        },
        {
                .name = "w",
                .type = CE_CDB_TYPE_FLOAT,

        },
};


static uint64_t _vec4_cdb_type() {
    return MATERIAL_VAR_TYPE_VEC4;
}

static void _vec4_property_editor(uint64_t obj,
                                  uint64_t context) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    const char *name = ce_cdb_a0->read_str(r, MATERIAL_VAR_NAME_PROP, "");

    ct_editor_ui_a0->prop_vec4(obj,
                               name,
                               (uint64_t[4]) {MATERIAL_VAR_VALUE_PROP_X,
                                              MATERIAL_VAR_VALUE_PROP_Y,
                                              MATERIAL_VAR_VALUE_PROP_Z,
                                              MATERIAL_VAR_VALUE_PROP_W},
                               (ui_vec4_p0) {});

}

static struct ct_property_editor_i0 vec4_property_editor_api = {
        .cdb_type = _vec4_cdb_type,
        .draw_ui = _vec4_property_editor,
};

static uint64_t _color_cdb_type() {
    return MATERIAL_VAR_TYPE_COLOR;
}

static void _color_property_editor(uint64_t obj,
                                   uint64_t context) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    const char *name = ce_cdb_a0->read_str(r, MATERIAL_VAR_NAME_PROP, "");

    ct_editor_ui_a0->prop_vec4(obj,
                               name,
                               (uint64_t[4]) {MATERIAL_VAR_VALUE_PROP_X,
                                              MATERIAL_VAR_VALUE_PROP_Y,
                                              MATERIAL_VAR_VALUE_PROP_Z,
                                              MATERIAL_VAR_VALUE_PROP_W},
                               (ui_vec4_p0) {.color=true});
}

static struct ct_property_editor_i0 color_property_editor_api = {
        .cdb_type = _color_cdb_type,
        .draw_ui = _color_property_editor,
};


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

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .db = ce_cdb_a0->db()
    };

    api->register_api(CT_MATERIAL_API, &material_api, sizeof(material_api));
    api->add_impl(CT_RESOURCE_I, &ct_resource_api, sizeof(ct_resource_api));
    api->add_impl(CT_PROPERTY_EDITOR_I, &_property_editor_api,
                  sizeof(_property_editor_api));

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &vec4_property_editor_api, sizeof(vec4_property_editor_api));

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &color_property_editor_api, sizeof(color_property_editor_api));

    ce_cdb_a0->reg_obj_type(MATERIAL_TYPE, material_prop, CE_ARRAY_LEN(material_prop));

    ce_cdb_a0->reg_obj_type(MATERIAL_RENDER_STATE, CE_ARR_ARG(render_state_prop));

    ce_cdb_a0->reg_obj_type(MATERIAL_LAYER_TYPE,
                            material_layer_prop, CE_ARRAY_LEN(material_layer_prop));

    ce_cdb_a0->reg_obj_type(MATERIAL_VAR_TYPE_TEXTURE,
                            material_texture_prop, CE_ARRAY_LEN(material_texture_prop));

    ce_cdb_a0->reg_obj_type(MATERIAL_VAR_TYPE_TEXTURE_HANDLER,
                            material_texture_handler_prop,
                            CE_ARRAY_LEN(material_texture_handler_prop));

    ce_cdb_a0->reg_obj_type(MATERIAL_VAR_TYPE_COLOR,
                            material_color_prop, CE_ARRAY_LEN(material_color_prop));

    ce_cdb_a0->reg_obj_type(MATERIAL_VAR_TYPE_VEC4,
                            material_vec4_prop, CE_ARRAY_LEN(material_vec4_prop));

}

void CE_MODULE_UNLOAD(material)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

}
