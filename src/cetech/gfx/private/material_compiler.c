//==============================================================================
// Include
//==============================================================================
#include <celib/ydb.h>
#include <celib/os.h>
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/hashlib.h"
#include <celib/array.inl>
#include <celib/yng.h>
#include <celib/cdb.h>

#include "cetech/ecs/ecs.h"
#include "cetech/machine/machine.h"
#include <bgfx/defines.h>
#include <cetech/gfx/renderer.h>
#include "cetech/resource/resource.h"

#include <cetech/gfx/material.h>
#include <cetech/resource/builddb.h>
#include <celib/buffer.inl>
#include "material.h"

#define _G material_compiler_globals

struct _G {
    struct ce_alloc *allocator;
} _G;

void _forach_variable_clb(const char *filename,
                          uint64_t root_key,
                          uint64_t key,
                          ce_cdb_obj_o *variables_obj) {

    uint64_t var_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(var_obj);

    const char *name = ce_cdb_a0->read_str(root_key, ce_id_a0->id64("name"), "");

    ce_cdb_a0->set_subobject(variables_obj, ce_id_a0->id64(name), var_obj);

    ce_cdb_a0->set_str(w, MATERIAL_VAR_NAME_PROP, name);

    char uniform_name[32];
    strcpy(uniform_name, name);

    const char *type = ce_cdb_a0->read_str(root_key, ce_id_a0->id64("type"), "");

    struct material_variable mat_var = {};


    if (!strcmp(type, "texture")) {
        uint64_t texture_name = 0;

        //TODO : None ptype?
        if (ce_cdb_a0->prop_exist(root_key,  ce_id_a0->id64("value"))) {
            const char *v = ce_cdb_a0->read_str(root_key, ce_id_a0->id64("value"), "");
            texture_name = ce_id_a0->id64(v);
        }

        mat_var.type = MAT_VAR_TEXTURE;
        mat_var.t = texture_name;

        ce_cdb_a0->set_uint64(w, MATERIAL_VAR_VALUE_PROP, mat_var.t);

    } else if (!strcmp(type, "vec4")) {
        mat_var.type = MAT_VAR_VEC4;
        ce_cdb_a0->read_vec4(root_key, ce_id_a0->id64("value"), mat_var.v4);
        ce_cdb_a0->set_vec3(w, MATERIAL_VAR_VALUE_PROP, mat_var.v3);
    } else if (!strcmp(type, "color")) {
        mat_var.type = MAT_VAR_COLOR4;
        ce_cdb_a0->read_vec4(root_key, ce_id_a0->id64("value"), mat_var.v4);
        ce_cdb_a0->set_vec4(w, MATERIAL_VAR_VALUE_PROP, mat_var.v4);

    } else if (!strcmp(type, "mat4")) {
        mat_var.type = MAT_VAR_MAT44;
        ce_cdb_a0->read_mat4(root_key, ce_id_a0->id64("value"), mat_var.m44);
        ce_cdb_a0->set_mat4(w, MATERIAL_VAR_VALUE_PROP, mat_var.m44);
    }

    ce_cdb_a0->set_uint64(w, MATERIAL_VAR_TYPE_PROP, mat_var.type);
    ce_cdb_a0->write_commit(w);
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


void foreach_layer(const char *filename,
                   uint64_t root_key,
                   uint64_t key,
                   uint64_t layer_obj) {

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(layer_obj);

    ce_cdb_a0->set_str(w, MATERIAL_LAYER_NAME, ce_id_a0->str_from_id64(key));

    const char *shader = ce_cdb_a0->read_str(root_key, ce_id_a0->id64("shader"), "");
    uint64_t shader_id = ce_id_a0->id64(shader);

    ce_cdb_a0->set_uint64(w, MATERIAL_SHADER_PROP, shader_id);

    if (ce_cdb_a0->prop_exist(root_key, ce_yng_a0->key("render_state"))) {
        uint64_t curent_render_state = 0;

        uint64_t render_state = ce_cdb_a0->read_subobject(root_key, ce_yng_a0->key("render_state"), 0);
        const uint64_t render_state_count = ce_cdb_a0->prop_count(render_state);
        uint64_t render_state_keys[render_state_count];
        ce_cdb_a0->prop_keys(render_state, render_state_keys);

        for (uint32_t i = 0; i < render_state_count; ++i) {
            curent_render_state |= render_state_to_enum(render_state_keys[i]);
        }

        ce_cdb_a0->set_uint64(w, MATERIAL_STATE_PROP, curent_render_state);
    }

    uint64_t variables_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
    ce_cdb_a0->set_subobject(w, MATERIAL_VARIABLES_PROP, variables_obj);

    if (ce_cdb_a0->prop_exist(root_key, ce_yng_a0->key("variables"))) {
        uint64_t variables = ce_cdb_a0->read_subobject(root_key, ce_yng_a0->key("variables"), 0);
        const uint64_t variables_keys_count = ce_cdb_a0->prop_count(variables);
        uint64_t variables_keys[variables_keys_count];
        ce_cdb_a0->prop_keys(variables, variables_keys);

        ce_cdb_obj_o *var_w = ce_cdb_a0->write_begin(variables_obj);

        for (uint32_t i = 0; i < variables_keys_count; ++i) {
            uint64_t _var = ce_cdb_a0->read_subobject(variables, variables_keys[i], 0);
            _forach_variable_clb(filename, _var, variables_keys[i], var_w);
        }
        ce_cdb_a0->write_commit(var_w);
    }

    ce_cdb_a0->write_commit(w);
};

void name_from_filename(const char *fullname,
                        char *name) {
    const char *resource_type = ce_os_a0->path->extension(fullname);
    size_t size = strlen(fullname) - strlen(resource_type) - 1;
    memcpy(name, fullname, size);
}

bool material_compiler(const char *filename,
                       uint64_t k,
                       struct ct_resource_id rid,
                       const char *fullname) {

    uint64_t obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), MATERIAL_TYPE);

    uint64_t layers_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
    ce_cdb_a0->set_subobject(w, MATERIAL_LAYERS, layers_obj);
    ce_cdb_a0->write_commit(w);

    if (ce_cdb_a0->prop_exist(k, ce_yng_a0->key("layers"))) {

        uint64_t layers = ce_cdb_a0->read_subobject(k, ce_yng_a0->key("layers"), 0);
        const uint64_t layers_keys_count = ce_cdb_a0->prop_count(layers);
        uint64_t layers_keys[layers_keys_count];
        ce_cdb_a0->prop_keys(layers, layers_keys);

        w = ce_cdb_a0->write_begin(layers_obj);
        for (uint32_t i = 0; i < layers_keys_count; ++i) {
            uint64_t layer_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
            ce_cdb_a0->set_subobject(w, layers_keys[i], layer_obj);

            uint64_t _layer = ce_cdb_a0->read_subobject(layers, layers_keys[i], 0);

            foreach_layer(filename, _layer, layers_keys[i], layer_obj);
        }
        ce_cdb_a0->write_commit(w);

//    name_from_filename(filename, resource.asset_name);

        char *output_blob = NULL;
        ce_cdb_a0->dump(obj, &output_blob, ce_memory_a0->system);
        ct_builddb_a0->put_resource(fullname, rid, filename, output_blob,
                                    ce_array_size(output_blob));

        ce_buffer_free(output_blob, _G.allocator);
    }

    ce_cdb_a0->destroy_object(obj);

    return true;
}

int materialcompiler_init(struct ce_api_a0 *api) {
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_os_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_yng_a0);
    CE_INIT_API(api, ce_ydb_a0);

    _G = (struct _G) {
            .allocator=ce_memory_a0->system
    };


    return 1;
}