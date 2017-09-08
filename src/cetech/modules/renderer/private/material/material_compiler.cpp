//==============================================================================
// Include
//==============================================================================

#include <celib/fpumath.h>
#include <bgfx/defines.h>
#include <cetech/kernel/ydb.h>
#include "celib/array.inl"
#include "celib/handler.inl"
#include "cetech/kernel/macros.h"

#include "cetech/kernel/vio.h"
#include "cetech/kernel/path.h"
#include "cetech/kernel/memory.h"
#include "cetech/kernel/api_system.h"
#include "cetech/kernel/hashlib.h"

#include "cetech/modules/entity/entity.h"
#include "cetech/modules/machine/machine.h"
#include "cetech/kernel/resource.h"

#include "material.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);

using namespace celib;

#include "material_blob.h"

namespace material_compiler {
    namespace {
        struct material_compile_output {
            Array<uint64_t> layer_names;
            Array<uint64_t> shader_name;
            Array<char> uniform_names;
            Array<uint32_t> uniform_count;
            Array<uint32_t> layer_offset;
            Array<material_variable> var;
            Array<uint64_t> render_state;
            uint64_t curent_render_state;
        };


        void _forach_variable_clb(const char *filename,
                                  uint64_t root_key,
                                  uint64_t key,
                                  material_compile_output &output) {

            uint64_t tmp_keys[] = {
                    root_key,
                    key,
                    ct_yng_a0.calc_key("name"),
            };

            const char *name = ct_ydb_a0.get_string(filename, tmp_keys,
                                                    CETECH_ARRAY_LEN(tmp_keys),
                                                    "");
            char uniform_name[32];
            strcpy(uniform_name, name);

            tmp_keys[2] = ct_yng_a0.calc_key("type");
            const char *type = ct_ydb_a0.get_string(filename, tmp_keys,
                                                    CETECH_ARRAY_LEN(tmp_keys),
                                                    "");

            material_variable mat_var = {};

            tmp_keys[2] = ct_yng_a0.calc_key("value");
            if (!strcmp(type, "texture")) {
                uint64_t texture_name = 0;

                //TODO : None type?
                if(ct_ydb_a0.has_key(filename, tmp_keys,
                                     CETECH_ARRAY_LEN(tmp_keys))) {
                        const char *v = ct_ydb_a0.get_string(
                                filename,
                                tmp_keys, CETECH_ARRAY_LEN(tmp_keys), "");
                    texture_name = ct_hash_a0.id64_from_str(v);
                }

                mat_var.type = MAT_VAR_TEXTURE;
                mat_var.t = texture_name;

            } else if (!strcmp(type, "vec4")) {
                mat_var.type = MAT_VAR_VEC4;
                ct_ydb_a0.get_vec4(filename, tmp_keys,
                                   CETECH_ARRAY_LEN(tmp_keys), mat_var.v4,
                                   (float[4]) {0.0f});

            } else if (!strcmp(type, "mat4")) {
                mat_var.type = MAT_VAR_MAT44;
                ct_ydb_a0.get_mat4(filename, tmp_keys,
                                   CETECH_ARRAY_LEN(tmp_keys), mat_var.m44,
                                   (float[16]) {0.0f});
            }

            array::push(output.var, &mat_var, 1);
            array::push(output.uniform_names, uniform_name, CETECH_ARRAY_LEN(uniform_name));
        }
    }

    uint64_t render_state_to_enum(uint64_t name) {

        static struct {
            uint64_t name;
            uint64_t e;
        } _tbl[] = {
                {.name = ct_hash_a0.id64_from_str(""), .e = 0},
                {.name = ct_hash_a0.id64_from_str("rgb_write"), .e = BGFX_STATE_RGB_WRITE},
                {.name = ct_hash_a0.id64_from_str("alpha_write"), .e = BGFX_STATE_ALPHA_WRITE},
                {.name = ct_hash_a0.id64_from_str("depth_write"), .e = BGFX_STATE_DEPTH_WRITE},
                {.name = ct_hash_a0.id64_from_str("depth_test_less"), .e = BGFX_STATE_DEPTH_TEST_LESS},
                {.name = ct_hash_a0.id64_from_str("cull_ccw"), .e = BGFX_STATE_CULL_CCW},
                {.name = ct_hash_a0.id64_from_str("msaa"), .e = BGFX_STATE_MSAA},
        };

        for (uint32_t i = 1; i < CETECH_ARRAY_LEN(_tbl); ++i) {
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
                ct_yng_a0.calc_key("shader"),
        };

        uint64_t tmp_key = ct_yng_a0.combine_key(tmp_keys,
                                                    CETECH_ARRAY_LEN(tmp_keys));

        const char *shader = ct_ydb_a0.get_string(filename, &tmp_key, 1, "");
        uint64_t shader_id = ct_hash_a0.id64_from_str(shader);
        array::push_back(output.shader_name, shader_id);

        auto layer_id = key;
        auto layer_offset = array::size(output.var);

        tmp_keys[2] = ct_yng_a0.calc_key("render_state");
        tmp_key = ct_yng_a0.combine_key(tmp_keys,
                                           CETECH_ARRAY_LEN(tmp_keys));
        if (ct_ydb_a0.has_key(filename, &tmp_key, 1)) {
            output.curent_render_state = 0;

            uint64_t render_state_keys[32] = {};
            uint32_t render_state_count = 0;

            ct_ydb_a0.get_map_keys(filename,
                                   &tmp_key, 1,
                                   render_state_keys,
                                   CETECH_ARRAY_LEN(render_state_keys),
                                   &render_state_count);

            for (uint32_t i = 0; i < render_state_count; ++i) {
                output.curent_render_state |= render_state_to_enum(render_state_keys[i]);
            }
        }

        array::push_back(output.layer_names, layer_id);
        array::push_back(output.layer_offset, layer_offset);
        array::push_back(output.render_state, output.curent_render_state);

        tmp_keys[2] = ct_yng_a0.calc_key("variables");
        tmp_key = ct_yng_a0.combine_key(tmp_keys,
                                           CETECH_ARRAY_LEN(tmp_keys));
        if (ct_ydb_a0.has_key(filename, &tmp_key, 1)) {
            uint64_t layers_keys[32] = {};
            uint32_t layers_keys_count = 0;

            ct_ydb_a0.get_map_keys(filename,
                                   &tmp_key, 1,
                                   layers_keys, CETECH_ARRAY_LEN(layers_keys),
                                   &layers_keys_count);

            for (uint32_t i = 0; i < layers_keys_count; ++i) {
                _forach_variable_clb(filename, tmp_key, layers_keys[i], output);
            }
        }

        array::push_back(output.uniform_count,
                         array::size(output.var) - layer_offset);

    };

    int compiler(const char *filename,
                 ct_vio *source_vio,
                 ct_vio *build_vio,
                 ct_compilator_api *compilator_api) {

        CEL_UNUSED(source_vio);
        CEL_UNUSED(compilator_api);

        struct material_compile_output output = {};
        output.uniform_names.init(ct_memory_a0.main_allocator());
        output.layer_names.init(ct_memory_a0.main_allocator());
        output.uniform_count.init(ct_memory_a0.main_allocator());
        output.var.init(ct_memory_a0.main_allocator());
        output.render_state.init(ct_memory_a0.main_allocator());
        output.layer_offset.init(ct_memory_a0.main_allocator());
        output.shader_name.init(ct_memory_a0.main_allocator());

        uint64_t key = ct_yng_a0.calc_key("layers");

        if (!ct_ydb_a0.has_key(filename, &key, 1)) {
            return 0;
        }

        uint64_t layers_keys[32] = {};
        uint32_t layers_keys_count = 0;

        ct_ydb_a0.get_map_keys(filename,
                               &key, 1,
                               layers_keys, CETECH_ARRAY_LEN(layers_keys),
                               &layers_keys_count);

        for (uint32_t i = 0; i < layers_keys_count; ++i) {
            foreach_layer(filename, key, layers_keys[i], output);
        }

        material_blob::blob_t resource = {
                .all_uniform_count = array::size(output.var),
                .layer_count = array::size(output.layer_names),
        };


        build_vio->write(build_vio->inst, &resource, sizeof(resource), 1);

        build_vio->write(build_vio->inst, array::begin(output.layer_names),
                         sizeof(uint64_t), array::size(output.layer_names));

        build_vio->write(build_vio->inst, array::begin(output.shader_name),
                         sizeof(uint64_t), array::size(output.shader_name));

        build_vio->write(build_vio->inst, array::begin(output.uniform_count),
                         sizeof(uint32_t), array::size(output.uniform_count));

        build_vio->write(build_vio->inst, array::begin(output.render_state),
                         sizeof(uint64_t), array::size(output.render_state));

        build_vio->write(build_vio->inst, array::begin(output.var),
                         sizeof(material_variable), array::size(output.var));

        build_vio->write(build_vio->inst, array::begin(output.uniform_names),
                         sizeof(char),
                         array::size(output.uniform_names));

        build_vio->write(build_vio->inst, array::begin(output.layer_offset),
                         sizeof(uint32_t), array::size(output.layer_offset));

        output.uniform_names.destroy();
        output.layer_names.destroy();
        output.uniform_count.destroy();
        output.var.destroy();
        output.layer_offset.destroy();
        output.shader_name.destroy();
        output.render_state.destroy();

        return 1;
    }

    int init(ct_api_a0 *api) {
        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_resource_a0);
        CETECH_GET_API(api, ct_path_a0);
        CETECH_GET_API(api, ct_vio_a0);
        CETECH_GET_API(api, ct_hash_a0);
        CETECH_GET_API(api, ct_yng_a0);
        CETECH_GET_API(api, ct_ydb_a0);

        ct_resource_a0.compiler_register(ct_hash_a0.id64_from_str("material"),
                                         compiler, true);

        return 1;
    }
}
