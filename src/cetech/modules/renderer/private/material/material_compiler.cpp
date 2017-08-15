#ifdef CETECH_CAN_COMPILE

//==============================================================================
// Include
//==============================================================================

#include <celib/fpumath.h>
#include <bgfx/defines.h>
#include "celib/array.inl"
#include "celib/handler.inl"
#include "cetech/core/macros.h"

#include "cetech/core/os/vio.h"
#include "cetech/core/os/path.h"
#include "cetech/core/os/errors.h"
#include "cetech/core/yaml/yaml.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/hashlib/hashlib.h"

#include "cetech/engine/entity/entity.h"
#include "cetech/engine/machine/machine.h"
#include "cetech/engine/resource/resource.h"

#include "material.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);

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

        void _preprocess(const char *filename,
                         yaml_node_t root,
                         ct_compilator_api *capi) {
            auto a = ct_memory_a0.main_allocator();

            yaml_node_t parent_node = yaml_get_node(root, "parent");

            if (yaml_is_valid(parent_node)) {
                char prefab_file[256] = {};
                char prefab_str[256] = {};
                yaml_as_string(parent_node, prefab_str,
                               CETECH_ARRAY_LEN(prefab_str));
                snprintf(prefab_file, CETECH_ARRAY_LEN(prefab_file),
                         "%s.material",
                         prefab_str);

                capi->add_dependency(filename, prefab_file);

                const char *source_dir = ct_resource_a0.compiler_get_source_dir();
                char *full_path = ct_path_a0.join(a, 2, source_dir,
                                                  prefab_file);

                ct_vio *prefab_vio = ct_vio_a0.from_file(full_path,
                                                         VIO_OPEN_READ);

                CEL_FREE(a, full_path);

                char prefab_data[prefab_vio->size(prefab_vio->inst) + 1];
                memset(prefab_data, 0, prefab_vio->size(prefab_vio->inst) + 1);
                prefab_vio->read(prefab_vio->inst, prefab_data, sizeof(char),
                                 prefab_vio->size(prefab_vio->inst));
                prefab_vio->close(prefab_vio->inst);

                yaml_document_t h;
                yaml_node_t prefab_root = yaml_load_str(prefab_data, &h);

                _preprocess(filename, prefab_root, capi);
                yaml_merge(root, prefab_root);
            }
        }

        void _forach_variable_clb(yaml_node_t key,
                                  yaml_node_t value,
                                  void *_data) {

            struct material_compile_output *output = (material_compile_output *) _data;

            char tmp_buffer[512] = {};
            char uniform_name[32] = {};

            yaml_as_string(key, uniform_name,
                           CETECH_ARRAY_LEN(uniform_name) - 1);

            material_variable mat_var = {};

            auto variable_type = yaml_node_type(value);

            if (YAML_TYPE_SCALAR == variable_type) {
                yaml_as_string(value, tmp_buffer, CETECH_ARRAY_LEN(tmp_buffer));
                uint64_t texture_name = ct_hash_a0.id64_from_str(tmp_buffer);

                mat_var.type = MAT_VAR_TEXTURE;
                mat_var.t = texture_name;

            }

            if (YAML_TYPE_SEQ == variable_type) {
                uint8_t count = yaml_node_size(value);
                if (4 == count) {
                    float v[4];
                    yaml_as_vec4(value, v);

                    mat_var.type = MAT_VAR_VEC4;

                    celib::vec4_move(mat_var.v4, v);

                } else if (16 == count) {
                    float m[16];
                    yaml_as_mat44(value, m);

                    mat_var.type = MAT_VAR_MAT44;
                    celib::mat4_move(mat_var.v4, m);
                }
            }

            array::push(output->var, &mat_var, 1);
            array::push(output->uniform_names, uniform_name,
                        CETECH_ARRAY_LEN(uniform_name));
        }
    }


    //        (0
//                          | BGFX_STATE_RGB_WRITE
//                          | BGFX_STATE_ALPHA_WRITE
//                          | BGFX_STATE_DEPTH_TEST_LESS
//                          | BGFX_STATE_DEPTH_WRITE
//                          | BGFX_STATE_CULL_CCW
//                          | BGFX_STATE_MSAA );

    uint64_t render_state_to_enum(const char* name) {
        static struct {
            const char* name;
            uint64_t  e;
        } _tbl[] = {
                {.name = "", .e = 0},
                {.name = "rgb_write", .e = BGFX_STATE_RGB_WRITE},
                {.name = "alpha_write", .e = BGFX_STATE_ALPHA_WRITE},
                {.name = "depth_write", .e = BGFX_STATE_DEPTH_WRITE},

                {.name = "depth_test_less", .e = BGFX_STATE_DEPTH_TEST_LESS},

                {.name = "cull_ccw", .e = BGFX_STATE_CULL_CCW},

                {.name = "msaa", .e = BGFX_STATE_MSAA},
        };

        for (int i = 1; i < CETECH_ARRAY_LEN(_tbl); ++i) {
            if (0 != strcmp(_tbl[i].name, name)) {
                continue;
            }

            return _tbl[i].e;
        }

        return _tbl[0].e;
    }


    int compiler(const char *filename,
                 ct_vio *source_vio,
                 ct_vio *build_vio,
                 ct_compilator_api *compilator_api) {

        char *source_data =
                CEL_ALLOCATE(ct_memory_a0.main_allocator(), char,
                             source_vio->size(source_vio->inst) + 1);
        memset(source_data, 0, source_vio->size(source_vio->inst) + 1);

        source_vio->read(source_vio->inst, source_data, sizeof(char),
                         source_vio->size(source_vio->inst));

        yaml_document_t h;
        yaml_node_t root = yaml_load_str(source_data, &h);

        _preprocess(filename, root, compilator_api);

        struct material_compile_output output = {};
        output.uniform_names.init(ct_memory_a0.main_allocator());
        output.layer_names.init(ct_memory_a0.main_allocator());
        output.uniform_count.init(ct_memory_a0.main_allocator());
        output.var.init(ct_memory_a0.main_allocator());
        output.render_state.init(ct_memory_a0.main_allocator());
        output.layer_offset.init(ct_memory_a0.main_allocator());
        output.shader_name.init(ct_memory_a0.main_allocator());

        yaml_node_t layers = yaml_get_node(root, "layers");
        if (!yaml_is_valid(layers)) {
            return 0;
        }

        yaml_node_foreach_dict(
                layers,
                [](yaml_node_t key,
                   yaml_node_t value,
                   void *_data) {

                    material_compile_output &output = *((material_compile_output *) _data);

                    yaml_node_t shader_node = yaml_get_node(value, "shader");
                    CETECH_ASSERT("material", yaml_is_valid(shader_node));

                    char tmp_buffer[256] = {};
                    yaml_as_string(shader_node, tmp_buffer,
                                   CETECH_ARRAY_LEN(tmp_buffer));

                    uint64_t shader_id = ct_hash_a0.id64_from_str(tmp_buffer);
                    array::push_back(output.shader_name, shader_id);


                    yaml_as_string(key, tmp_buffer,
                                   CETECH_ARRAY_LEN(tmp_buffer));

                    auto layer_id = ct_hash_a0.id64_from_str(tmp_buffer);
                    auto layer_offset = array::size(output.var);

                    yaml_node_t render_state = yaml_get_node(value, "render_state");
                    if (yaml_is_valid(render_state)) {
                        output.curent_render_state = 0;

                        yaml_node_foreach_dict(
                                render_state,
                                [](yaml_node_t key,
                                   yaml_node_t value,
                                   void *_data) {

                                    material_compile_output &output = *((material_compile_output *) _data);

                                    char state_name[64] = {};
                                    yaml_as_string(key, state_name,
                                                   CETECH_ARRAY_LEN(state_name));

                                    output.curent_render_state |= render_state_to_enum(state_name);

                                }, &output);
                    }

                    array::push_back(output.layer_names, layer_id);
                    array::push_back(output.layer_offset, layer_offset);
                    array::push_back(output.render_state, output.curent_render_state);

                    yaml_node_t variables = yaml_get_node(value, "variables");
                    if (yaml_is_valid(variables)) {
                        yaml_node_foreach_dict(variables, _forach_variable_clb,
                                               &output);
                    }

                    array::push_back(output.uniform_count,
                                     array::size(output.var) - layer_offset);

                }, &output);

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

        CEL_FREE(ct_memory_a0.main_allocator(), source_data);
        return 1;
    }

    int init(ct_api_a0 *api) {
        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_resource_a0);
        CETECH_GET_API(api, ct_path_a0);
        CETECH_GET_API(api, ct_vio_a0);
        CETECH_GET_API(api, ct_hash_a0);

        ct_resource_a0.compiler_register(ct_hash_a0.id64_from_str("material"),
                                         compiler);

        return 1;
    }
}

#endif