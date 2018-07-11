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

#include "material.h"


#include "material_blob.h"

struct material_compile_output {
    uint64_t *layer_names;
    uint64_t *shader_name;
    char *uniform_names;
    uint32_t *uniform_count;
    uint32_t *layer_offset;
    material_variable *var;
    uint64_t *render_state;
    uint64_t curent_render_state;
};

#define _G material_compiler_globals

struct _G {
    ct_alloc *allocator;
} _G;

void _forach_variable_clb(const char *filename,
                          uint64_t root_key,
                          uint64_t key,
                          material_compile_output &output) {
    uint64_t tmp_keys[] = {
            root_key,
            key,
            ct_yng_a0->key("name"),
    };

    const char *name = ct_ydb_a0->get_str(filename, tmp_keys,
                                          CT_ARRAY_LEN(tmp_keys), "");
    char uniform_name[32];
    strcpy(uniform_name, name);

    tmp_keys[2] = ct_yng_a0->key("type");
    const char *type = ct_ydb_a0->get_str(filename, tmp_keys,
                                          CT_ARRAY_LEN(tmp_keys), "");

    material_variable mat_var = {};

    tmp_keys[2] = ct_yng_a0->key("value");
    if (!strcmp(type, "texture")) {
        uint64_t texture_name = 0;

        //TODO : None ptype?
        if (ct_ydb_a0->has_key(filename, tmp_keys, CT_ARRAY_LEN(tmp_keys))) {
            const char *v = ct_ydb_a0->get_str(filename,
                                               tmp_keys,
                                               CT_ARRAY_LEN(tmp_keys),
                                               "");
            texture_name = CT_ID32_0(v);
        }

        mat_var.type = MAT_VAR_TEXTURE;
        mat_var.t = texture_name;

    } else if (!strcmp(type, "vec4")) {
        mat_var.type = MAT_VAR_VEC4;
        ct_ydb_a0->get_vec4(filename, tmp_keys,
                            CT_ARRAY_LEN(tmp_keys), mat_var.v4,
                            (float[4]) {0.0f});
    } else if (!strcmp(type, "color")) {
        mat_var.type = MAT_VAR_COLOR4;
        ct_ydb_a0->get_vec4(filename, tmp_keys,
                            CT_ARRAY_LEN(tmp_keys), mat_var.v4,
                            (float[4]) {0.0f});

    } else if (!strcmp(type, "mat4")) {
        mat_var.type = MAT_VAR_MAT44;
        ct_ydb_a0->get_mat4(filename, tmp_keys,
                            CT_ARRAY_LEN(tmp_keys), mat_var.m44,
                            (float[16]) {0.0f});
    }

    ct_array_push_n(output.var, &mat_var, 1, _G.allocator);
    ct_array_push_n(output.uniform_names, uniform_name,
                    CT_ARRAY_LEN(uniform_name), _G.allocator);
}

#define RENDER_STATE_RGB_WRITE \
    CT_ID64_0("rgb_write", 0xdad21ff8b23271ffULL)

#define RENDER_STATE_ALPHA_WRITE \
    CT_ID64_0("alpha_write", 0x93c0953aa6e40b10ULL)

#define RENDER_STATE_DEPTH_WRITE \
    CT_ID64_0("depth_write", 0x6d5cef63be1e7b46ULL)

#define RENDER_STATE_DEPTH_TEST_LESS \
    CT_ID64_0("depth_test_less", 0x25d531ce0f04418eULL)

#define RENDER_STATE_CULL_CCW \
    CT_ID64_0("cull_ccw", 0x8447d75aa845c612ULL)

#define RENDER_STATE_CULL_CW \
    CT_ID64_0("cull_cw", 0x6b5530bb1cba7b79ULL)

#define RENDER_STATE_MSAA \
    CT_ID64_0("msaa", 0xdc0268c3aab08183ULL)

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

    return _tbl[0].e;
}


void foreach_layer(const char *filename,
                   uint64_t root_key,
                   uint64_t key,
                   material_compile_output &output) {
    uint64_t tmp_keys[] = {
            root_key,
            key,
            ct_yng_a0->key("shader"),
    };

    uint64_t tmp_key = ct_yng_a0->combine_key(tmp_keys,
                                              CT_ARRAY_LEN(tmp_keys));

    const char *shader = ct_ydb_a0->get_str(filename, &tmp_key, 1, "");
    uint64_t shader_id = CT_ID32_0(shader);
    ct_array_push(output.shader_name, shader_id, _G.allocator);

    auto layer_id = key;
    auto layer_offset = ct_array_size(output.var);

    tmp_keys[2] = ct_yng_a0->key("render_state");
    tmp_key = ct_yng_a0->combine_key(tmp_keys,
                                     CT_ARRAY_LEN(tmp_keys));
    if (ct_ydb_a0->has_key(filename, &tmp_key, 1)) {
        output.curent_render_state = 0;

        uint64_t render_state_keys[32] = {};
        uint32_t render_state_count = 0;

        ct_ydb_a0->get_map_keys(filename,
                                &tmp_key, 1,
                                render_state_keys,
                                CT_ARRAY_LEN(render_state_keys),
                                &render_state_count);

        for (uint32_t i = 0; i < render_state_count; ++i) {
            output.curent_render_state |= render_state_to_enum(
                    render_state_keys[i]);
        }
    }

    ct_array_push(output.layer_names, layer_id, _G.allocator);
    ct_array_push(output.layer_offset, layer_offset, _G.allocator);
    ct_array_push(output.render_state, output.curent_render_state,
                  _G.allocator);

    tmp_keys[2] = ct_yng_a0->key("variables");
    tmp_key = ct_yng_a0->combine_key(tmp_keys,
                                     CT_ARRAY_LEN(tmp_keys));
    if (ct_ydb_a0->has_key(filename, &tmp_key, 1)) {
        uint64_t layers_keys[32] = {};
        uint32_t layers_keys_count = 0;

        ct_ydb_a0->get_map_keys(filename,
                                &tmp_key, 1,
                                layers_keys, CT_ARRAY_LEN(layers_keys),
                                &layers_keys_count);

        for (uint32_t i = 0; i < layers_keys_count; ++i) {
            _forach_variable_clb(filename, tmp_key, layers_keys[i], output);
        }
    }

    ct_array_push(output.uniform_count,
                  ct_array_size(output.var) - layer_offset, _G.allocator);

};

void name_from_filename(const char *fullname,
                        char *name) {
    const char *resource_type = ct_os_a0->path->extension(fullname);
    size_t size = strlen(fullname) - strlen(resource_type) - 1;
    memcpy(name, fullname, size);
}

void material_compiler(const char *filename,
                       char **output_blob,
                       ct_compilator_api *compilator_api) {
    CT_UNUSED(compilator_api);

    struct material_compile_output output = {};

    uint64_t key = ct_yng_a0->key("layers");

    if (!ct_ydb_a0->has_key(filename, &key, 1)) {
        return;
    }

    uint64_t layers_keys[32] = {};
    uint32_t layers_keys_count = 0;

    ct_ydb_a0->get_map_keys(filename,
                            &key, 1,
                            layers_keys, CT_ARRAY_LEN(layers_keys),
                            &layers_keys_count);

    for (uint32_t i = 0; i < layers_keys_count; ++i) {
        foreach_layer(filename, key, layers_keys[i], output);
    }

    material_blob::blob_t resource = {
            .all_uniform_count = ct_array_size(output.var),
            .layer_count = ct_array_size(output.layer_names),
    };

    name_from_filename(filename, resource.asset_name);

    ct_array_push_n(*output_blob, &resource, sizeof(resource), _G.allocator);

    ct_array_push_n(*output_blob, output.layer_names,
                    sizeof(uint64_t) * ct_array_size(output.layer_names),
                    _G.allocator);

    ct_array_push_n(*output_blob, output.shader_name,
                    sizeof(uint64_t) * ct_array_size(output.shader_name),
                    _G.allocator);

    ct_array_push_n(*output_blob, output.uniform_count,
                    sizeof(uint32_t) * ct_array_size(output.uniform_count),
                    _G.allocator);

    ct_array_push_n(*output_blob, output.render_state,
                    sizeof(uint64_t) * ct_array_size(output.render_state),
                    _G.allocator);

    ct_array_push_n(*output_blob, output.var,
                    sizeof(material_variable) * ct_array_size(output.var),
                    _G.allocator);

    ct_array_push_n(*output_blob, output.uniform_names,
                    sizeof(char) * ct_array_size(output.uniform_names),
                    _G.allocator);

    ct_array_push_n(*output_blob, output.layer_offset,
                    sizeof(uint32_t) * ct_array_size(output.layer_offset),
                    _G.allocator);

    ct_array_free(output.uniform_names, _G.allocator);
    ct_array_free(output.layer_names, _G.allocator);
    ct_array_free(output.uniform_count, _G.allocator);
    ct_array_free(output.var, _G.allocator);
    ct_array_free(output.layer_offset, _G.allocator);
    ct_array_free(output.shader_name, _G.allocator);
    ct_array_free(output.render_state, _G.allocator);
}

int materialcompiler_init(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_os_a0);
    CETECH_GET_API(api, ct_hashlib_a0);
    CETECH_GET_API(api, ct_yng_a0);
    CETECH_GET_API(api, ct_ydb_a0);

    _G = (struct _G) {
            .allocator=ct_memory_a0->system
    };


    return 1;
}
