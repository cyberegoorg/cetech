//==============================================================================
// Include
//==============================================================================
#include <corelib/ydb.h>
#include <corelib/os.h>
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/hashlib.h"
#include "cetech/resource/resource.h"

#include "cetech/ecs/ecs.h"
#include "cetech/machine/machine.h"

#include <bgfx/defines.h>
#include <corelib/array.inl>
#include <corelib/yng.h>
#include <corelib/cdb.h>
#include <cetech/gfx/renderer.h>

#include <cetech/gfx/material.h>
#include "material.h"

#define _G material_compiler_globals

struct _G {
    struct ct_alloc *allocator;
} _G;

void _forach_variable_clb(const char *filename,
                          uint64_t root_key,
                          uint64_t key,
                          ct_cdb_obj_o *variables_obj) {

    uint64_t var_obj = ct_cdb_a0->create_object(ct_cdb_a0->db(), 0);

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(var_obj);

    uint64_t tmp_keys[] = {
            root_key,
            key,
            ct_yng_a0->key("name"),
    };

    const char *name = ct_ydb_a0->get_str(filename, tmp_keys,
                                          CT_ARRAY_LEN(tmp_keys), "");

    ct_cdb_a0->set_subobject(variables_obj, ct_hashlib_a0->id64(name), var_obj);

    ct_cdb_a0->set_str(w, MATERIAL_VAR_NAME_PROP, name);

    char uniform_name[32];
    strcpy(uniform_name, name);

    tmp_keys[2] = ct_yng_a0->key("type");
    const char *type = ct_ydb_a0->get_str(filename, tmp_keys,
                                          CT_ARRAY_LEN(tmp_keys), "");

    struct material_variable mat_var = {};

    tmp_keys[2] = ct_yng_a0->key("value");
    if (!strcmp(type, "texture")) {
        uint64_t texture_name = 0;

        //TODO : None ptype?
        if (ct_ydb_a0->has_key(filename, tmp_keys, CT_ARRAY_LEN(tmp_keys))) {
            const char *v = ct_ydb_a0->get_str(filename,
                                               tmp_keys,
                                               CT_ARRAY_LEN(tmp_keys),
                                               "");
            texture_name = ct_hashlib_a0->id64(v);
        }

        mat_var.type = MAT_VAR_TEXTURE;
        mat_var.t = texture_name;

        ct_cdb_a0->set_uint64(w, MATERIAL_VAR_VALUE_PROP, mat_var.t);

    } else if (!strcmp(type, "vec4")) {
        mat_var.type = MAT_VAR_VEC4;
        ct_ydb_a0->get_vec4(filename, tmp_keys,
                            CT_ARRAY_LEN(tmp_keys), mat_var.v4,
                            (float[4]) {0.0f});
        ct_cdb_a0->set_vec3(w, MATERIAL_VAR_VALUE_PROP, mat_var.v3);
    } else if (!strcmp(type, "color")) {
        mat_var.type = MAT_VAR_COLOR4;
        ct_ydb_a0->get_vec4(filename, tmp_keys,
                            CT_ARRAY_LEN(tmp_keys), mat_var.v4,
                            (float[4]) {0.0f});
        ct_cdb_a0->set_vec4(w, MATERIAL_VAR_VALUE_PROP, mat_var.v4);

    } else if (!strcmp(type, "mat4")) {
        mat_var.type = MAT_VAR_MAT44;
        ct_ydb_a0->get_mat4(filename, tmp_keys,
                            CT_ARRAY_LEN(tmp_keys), mat_var.m44,
                            (float[16]) {0.0f});
        ct_cdb_a0->set_mat4(w, MATERIAL_VAR_VALUE_PROP, mat_var.m44);
    }

    ct_cdb_a0->set_uint64(w, MATERIAL_VAR_TYPE_PROP, mat_var.type);
    ct_cdb_a0->write_commit(w);
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

    for (uint32_t i = 1; i < CT_ARRAY_LEN(_tbl); ++i) {
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

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(layer_obj);

    ct_cdb_a0->set_str(w, MATERIAL_LAYER_NAME, ct_yng_a0->get_key(key));

    uint64_t tmp_keys[] = {
            root_key,
            key,
            ct_yng_a0->key("shader"),
    };

    uint64_t tmp_key = ct_yng_a0->combine_key(tmp_keys,
                                              CT_ARRAY_LEN(tmp_keys));

    const char *shader = ct_ydb_a0->get_str(filename, &tmp_key, 1, "");
    uint64_t shader_id = ct_hashlib_a0->id64(shader);

    ct_cdb_a0->set_uint64(w, MATERIAL_SHADER_PROP, shader_id);

    tmp_keys[2] = ct_yng_a0->key("render_state");
    tmp_key = ct_yng_a0->combine_key(tmp_keys, CT_ARRAY_LEN(tmp_keys));
    if (ct_ydb_a0->has_key(filename, &tmp_key, 1)) {
        uint64_t curent_render_state = 0;

        uint64_t render_state_keys[32] = {};
        uint32_t render_state_count = 0;

        ct_ydb_a0->get_map_keys(filename,
                                &tmp_key, 1,
                                render_state_keys,
                                CT_ARRAY_LEN(render_state_keys),
                                &render_state_count);

        for (uint32_t i = 0; i < render_state_count; ++i) {
            curent_render_state |= render_state_to_enum(render_state_keys[i]);
        }

        ct_cdb_a0->set_uint64(w, MATERIAL_STATE_PROP, curent_render_state);
    }

    uint64_t variables_obj = ct_cdb_a0->create_object(ct_cdb_a0->db(), 0);
    ct_cdb_a0->set_subobject(w, MATERIAL_VARIABLES_PROP, variables_obj);

    tmp_keys[2] = ct_yng_a0->key("variables");
    tmp_key = ct_yng_a0->combine_key(tmp_keys, CT_ARRAY_LEN(tmp_keys));
    if (ct_ydb_a0->has_key(filename, &tmp_key, 1)) {
        uint64_t layers_keys[32] = {};
        uint32_t layers_keys_count = 0;

        ct_ydb_a0->get_map_keys(filename,
                                &tmp_key, 1,
                                layers_keys, CT_ARRAY_LEN(layers_keys),
                                &layers_keys_count);

        ct_cdb_obj_o *var_w = ct_cdb_a0->write_begin(variables_obj);

        for (uint32_t i = 0; i < layers_keys_count; ++i) {
            _forach_variable_clb(filename, tmp_key, layers_keys[i], var_w);
        }
        ct_cdb_a0->write_commit(var_w);
    }

    ct_cdb_a0->write_commit(w);
};

void name_from_filename(const char *fullname,
                        char *name) {
    const char *resource_type = ct_os_a0->path->extension(fullname);
    size_t size = strlen(fullname) - strlen(resource_type) - 1;
    memcpy(name, fullname, size);
}

void material_compiler(const char *filename,
                       char **output_blob) {

    uint64_t obj = ct_cdb_a0->create_object(ct_cdb_a0->db(), MATERIAL_TYPE);

    uint64_t layers_obj = ct_cdb_a0->create_object(ct_cdb_a0->db(), 0);

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(obj);
    ct_cdb_a0->set_subobject(w, MATERIAL_LAYERS, layers_obj);
    ct_cdb_a0->write_commit(w);

    uint64_t key = ct_yng_a0->key("layers");

    if (ct_ydb_a0->has_key(filename, &key, 1)) {

        uint64_t layers_keys[32] = {};
        uint32_t layers_keys_count = 0;

        ct_ydb_a0->get_map_keys(filename,
                                &key, 1,
                                layers_keys, CT_ARRAY_LEN(layers_keys),
                                &layers_keys_count);

        w = ct_cdb_a0->write_begin(layers_obj);
        for (uint32_t i = 0; i < layers_keys_count; ++i) {
            uint64_t layer_obj = ct_cdb_a0->create_object(ct_cdb_a0->db(), 0);
            ct_cdb_a0->set_subobject(w, layers_keys[i], layer_obj);
            foreach_layer(filename, key, layers_keys[i], layer_obj);
        }
        ct_cdb_a0->write_commit(w);

//    name_from_filename(filename, resource.asset_name);
        ct_cdb_a0->dump(obj, output_blob, ct_memory_a0->system);
    }

    ct_cdb_a0->destroy_object(obj);
}

int materialcompiler_init(struct ct_api_a0 *api) {
    CT_INIT_API(api, ct_memory_a0);
    CT_INIT_API(api, ct_resource_a0);
    CT_INIT_API(api, ct_os_a0);
    CT_INIT_API(api, ct_hashlib_a0);
    CT_INIT_API(api, ct_yng_a0);
    CT_INIT_API(api, ct_ydb_a0);

    _G = (struct _G) {
            .allocator=ct_memory_a0->system
    };


    return 1;
}
