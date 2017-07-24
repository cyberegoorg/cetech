#ifdef CETECH_CAN_COMPILE

//==============================================================================
// Include
//==============================================================================

#include <include/assimp/scene.h>
#include <include/assimp/postprocess.h>
#include <include/assimp/cimport.h>

#include <celib/allocator.h>
#include <celib/map.inl>

#include <cetech/core/hash.h>
#include <cetech/core/memory.h>
#include <cetech/core/api_system.h>
#include <cetech/machine/machine.h>


#include <cetech/engine/resource.h>
#include <cetech/engine/entity.h>
#include <cetech/core/yaml.h>

#include "cetech/engine/scenegraph.h"

#include "scene_blob.h"
#include <cetech/core/os/path.h>
#include <cetech/core/os/thread.h>
#include <cetech/core/os/vio.h>

using namespace celib;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_scenegprah_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_thread_a0);


namespace scene_resource_compiler {
    static const struct {
        const char *name;
        bgfx::Attrib::Enum attrib;
    } _chanel_types[] = {
            {.name="position", .attrib=bgfx::Attrib::Position},
            {.name="normal", .attrib=bgfx::Attrib::Normal},
            {.name="tangent", .attrib=bgfx::Attrib::Tangent},
            {.name="bitangent", .attrib=bgfx::Attrib::Bitangent},
            {.name="color0", .attrib=bgfx::Attrib::Color0},
            {.name="color1", .attrib=bgfx::Attrib::Color1},
            {.name="indices", .attrib=bgfx::Attrib::Indices},
            {.name="weight", .attrib=bgfx::Attrib::Weight},
            {.name="texcoord0", .attrib=bgfx::Attrib::TexCoord0},
            {.name="texcoord1", .attrib=bgfx::Attrib::TexCoord1},
            {.name="texcoord2", .attrib=bgfx::Attrib::TexCoord2},
            {.name="texcoord3", .attrib=bgfx::Attrib::TexCoord3},
            {.name="texcoord4", .attrib=bgfx::Attrib::TexCoord4},
            {.name="texcoord5", .attrib=bgfx::Attrib::TexCoord5},
            {.name="texcoord6", .attrib=bgfx::Attrib::TexCoord6},
            {.name="texcoord7", .attrib=bgfx::Attrib::TexCoord7},
    };

    static const struct {
        const char *name;
        size_t size;
        bgfx::AttribType::Enum attrib_type;
    } _attrin_tbl[] = {
            {.name="f32", .size=sizeof(float), .attrib_type=bgfx::AttribType::Float},
            {.name="int16_t", .size=sizeof(int16_t), .attrib_type=bgfx::AttribType::Int16},
            {.name="uint8_t", .size=sizeof(uint8_t), .attrib_type=bgfx::AttribType::Uint8},
            // TODO: {.name="f16", .size=sizeof(f16), .attrib_type=BGFX_ATTRIB_TYPE_HALF},
            // TODO: {.name="u10", .size=sizeof(u10), .attrib_type=BGFX_ATTRIB_TYPE_UINT10},
    };

    struct compile_output {
        Array<uint64_t> geom_name;
        Array<uint32_t> ib_offset;
        Array<uint32_t> vb_offset;
        Array<bgfx::VertexDecl> vb_decl;
        Array<uint32_t> ib_size;
        Array<uint32_t> vb_size;
        Array<uint32_t> ib;
        Array<uint8_t> vb;

        Array<uint64_t> node_name;
        Array<uint32_t> node_parent;
        Array<float> node_pose;
        Array<uint64_t> geom_node;
    };

    struct compile_output *_crete_compile_output() {
        cel_alloc *a = ct_memory_a0.main_allocator();
        struct compile_output *output =
                CEL_ALLOCATE(a, struct compile_output,
                                sizeof(struct compile_output));

        output->geom_name.init(a);
        output->ib_offset.init(a);
        output->vb_offset.init(a);
        output->vb_decl.init(a);
        output->ib_size.init(a);
        output->vb_size.init(a);
        output->ib.init(a);
        output->vb.init(a);

        output->node_name.init(a);
        output->geom_node.init(a);
        output->node_parent.init(a);
        output->node_pose.init(a);


        return output;
    }

    void _destroy_compile_output(struct compile_output *output) {
        cel_alloc *a = ct_memory_a0.main_allocator();

        output->geom_name.destroy();
        output->ib_offset.destroy();
        output->vb_offset.destroy();
        output->vb_decl.destroy();
        output->ib_size.destroy();
        output->vb_size.destroy();
        output->ib.destroy();
        output->vb.destroy();

        output->node_name.destroy();
        output->geom_node.destroy();
        output->node_parent.destroy();
        output->node_pose.destroy();

        CEL_FREE(a, output);
    }

    static void _type_to_attr_type(const char *name,
                                   bgfx::AttribType::Enum *attr_type,
                                   size_t *size) {

        for (int i = 0; i < CETECH_ARRAY_LEN(_attrin_tbl); ++i) {
            if (strcmp(_attrin_tbl[i].name, name) != 0) {
                continue;
            }


            *attr_type = _attrin_tbl[i].attrib_type;
            *size = _attrin_tbl[i].size;
            return;
        }

        *attr_type = bgfx::AttribType::Count;
        *size = 0;
    }

    void _parse_vertex_decl(bgfx::VertexDecl *decl,
                            uint32_t *vertex_size,
                            bgfx::Attrib::Enum type,
                            yaml_node_t decl_node) {
        yaml_node_t type_n = yaml_get_node(decl_node, "type");
        yaml_node_t size_n = yaml_get_node(decl_node, "size");

        char type_str[64] = {0};
        yaml_as_string(type_n, type_str, CETECH_ARRAY_LEN(type_str));

        bgfx::AttribType::Enum attrib_type;
        size_t v_size;

        _type_to_attr_type(type_str, &attrib_type, &v_size);

        *vertex_size += yaml_as_int(size_n) * v_size;

        decl->add(type, (uint8_t) yaml_as_int(size_n),
                  attrib_type,
                  0, 0);
    }


    static void _parese_types(bgfx::VertexDecl *decl,
                              yaml_node_t types,
                              uint32_t *vertex_size) {

        for (int i = 0; i < CETECH_ARRAY_LEN(_chanel_types); ++i) {
            YAML_NODE_SCOPE(node, types, _chanel_types[i].name,
                            if (yaml_is_valid(node))
                                _parse_vertex_decl(decl, vertex_size,
                                                   _chanel_types[i].attrib,
                                                   node););

        }
    }

    void _write_chanel(yaml_node_t node,
                       yaml_node_t types,
                       size_t i,
                       const char *name,
                       yaml_node_t chanels_n,
                       struct compile_output *output) {
        char tmp_buff[64] = {0};
        bgfx::AttribType::Enum attrib_type;
        size_t v_size;

        yaml_node_t idx_n = yaml_get_seq_node(node, i);
        uint32_t idx = yaml_as_int(idx_n);
        yaml_node_free(idx_n);

        uint32_t size = 0;

        {
            yaml_node_t chan_def_n = yaml_get_node(types, name);
            yaml_node_t type_n = yaml_get_node(chan_def_n, "type");
            yaml_node_t size_n = yaml_get_node(chan_def_n, "size");

            yaml_as_string(type_n, tmp_buff, CETECH_ARRAY_LEN(tmp_buff));
            size = yaml_as_int(size_n);

            yaml_node_free(chan_def_n);
            yaml_node_free(type_n);
            yaml_node_free(size_n);
        }

        _type_to_attr_type(tmp_buff, &attrib_type, &v_size);

        yaml_node_t chanel_data_n = yaml_get_node(chanels_n, name);
        for (int k = 0; k < size; ++k) {
            yaml_node_t n = yaml_get_seq_node(chanel_data_n, (idx * size) + k);

            // TODO: type
            float v = yaml_as_float(n);
            yaml_node_free(n);

            //log_debug("casdsadsa", "%s:%d -  %f", name, k, v);

            array::push(output->vb, (uint8_t *) &v, sizeof(v));
        }
        yaml_node_free(chanel_data_n);
    }

    void foreach_geometries_clb(yaml_node_t key,
                                yaml_node_t value,
                                void *_data) {
        struct compile_output *output = (compile_output *) _data;

        char name_str[64] = {0};
        yaml_as_string(key, name_str, CETECH_ARRAY_LEN(name_str));

        uint64_t name = ct_hash_a0.id64_from_str(name_str);

        array::push_back(output->geom_name, name);
        array::push_back<uint64_t>(output->geom_node, 0);
        array::push_back(output->ib_offset, array::size(output->ib));
        array::push_back(output->vb_offset, array::size(output->vb));

        // DECL
        bgfx::VertexDecl vertex_decl;
        vertex_decl.begin();

        yaml_node_t types = yaml_get_node(value, "types");
        uint32_t vertex_size = 0;
        _parese_types(&vertex_decl, types, &vertex_size);

        vertex_decl.end();
        array::push_back(output->vb_decl, vertex_decl);

        // IB, VB
        yaml_node_t chanels_n = yaml_get_node(value, "chanels");
        yaml_node_t indices_n = yaml_get_node(value, "indices");
        yaml_node_t i_size = yaml_get_node(indices_n, "size");

        uint32_t vertex_count = yaml_as_int(i_size);

        array::push_back(output->ib_size, vertex_count);
        array::push_back(output->vb_size, vertex_size * vertex_count);

        for (int i = 0; i < vertex_count; ++i) {
            for (int j = 0; j < CETECH_ARRAY_LEN(_chanel_types); ++j) {
                const char *name = _chanel_types[j].name;

                YAML_NODE_SCOPE(node, indices_n, name,
                                if (yaml_is_valid(node))
                                    _write_chanel(node, types, i, name,
                                                  chanels_n,
                                                  output););
            }

            array::push_back(output->ib, (uint32_t) i);
        }
    }


    struct foreach_graph_data {
        struct compile_output *output;
        uint32_t parent_idx;
    };

    void foreach_graph_clb(yaml_node_t key,
                           yaml_node_t value,
                           void *_data) {
        char buffer[128] = {0};
        struct foreach_graph_data *output = (foreach_graph_data *) _data;

        yaml_as_string(key, buffer, CETECH_ARRAY_LEN(buffer));
        uint64_t node_name = ct_hash_a0.id64_from_str(buffer);

        yaml_node_t local_pose = yaml_get_node(value, "local");

        float pose[16];
        yaml_as_mat44(local_pose, pose);

        uint32_t idx = (uint32_t) array::size(output->output->node_name);

        array::push_back(output->output->node_name, node_name);
        array::push_back(output->output->node_parent, output->parent_idx);
        array::push(output->output->node_pose, pose, 16);

        yaml_node_t geometries_n = yaml_get_node(value, "geometries");
        if (yaml_is_valid(geometries_n)) {
            const size_t name_count = yaml_node_size(geometries_n);
            for (int i = 0; i < name_count; ++i) {
                yaml_node_t name_node = yaml_get_seq_node(geometries_n, i);
                yaml_as_string(name_node, buffer, CETECH_ARRAY_LEN(buffer));
                yaml_node_free(name_node);

                uint64_t geom_name = ct_hash_a0.id64_from_str(buffer);
                for (int j = 0;
                     j < array::size(output->output->geom_name); ++j) {
                    if (geom_name != output->output->geom_name[j]) {
                        continue;
                    }

                    output->output->geom_node[j] = node_name;
                    break;
                }

            }
        }

        yaml_node_t children = yaml_get_node(value, "children");
        if (yaml_is_valid(children)) {
            struct foreach_graph_data graph_data = {
                    .parent_idx = idx,
                    .output = output->output
            };

            yaml_node_foreach_dict(children, foreach_graph_clb, &graph_data);
        }
    }

    int _compile_yaml(yaml_node_t root,
                      struct compile_output *output) {
        yaml_node_t geometries = yaml_get_node(root, "geometries");
        yaml_node_t graph = yaml_get_node(root, "graph");

        yaml_node_foreach_dict(geometries, foreach_geometries_clb, output);

        struct foreach_graph_data graph_data = {
                .parent_idx = UINT32_MAX,
                .output = output
        };

        yaml_node_foreach_dict(graph, foreach_graph_clb, &graph_data);
        return 1;
    }

    void _compile_assimp_node(struct aiNode *root,
                              uint32_t parent,
                              struct compile_output *output) {
        uint64_t name = ct_hash_a0.id64_from_str(root->mName.data);

        uint32_t idx = array::size(output->node_name);

        array::push_back(output->node_name, name);
        array::push_back(output->node_parent, parent);
        array::push(output->node_pose, &root->mTransformation.a1, 16);

        for (int i = 0; i < root->mNumChildren; ++i) {
            _compile_assimp_node(root->mChildren[i], idx, output);
        }

        for (int i = 0; i < root->mNumMeshes; ++i) {
            array::push_back(output->geom_node, name);
        }
    }

    int _compile_assimp(const char *filename,
                        yaml_node_t root,
                        struct compile_output *output,
                        ct_compilator_api *capi) {
        auto a = ct_memory_a0.main_allocator();

        yaml_node_t import_n = yaml_get_node(root, "import");
        yaml_node_t input_n = yaml_get_node(import_n, "input");

        yaml_node_t postprocess_n = yaml_get_node(import_n, "postprocess");

        char input_str[64] = {0};
        yaml_as_string(input_n, input_str, CETECH_ARRAY_LEN(input_str));
        capi->add_dependency(filename, input_str);

        const char *source_dir = ct_resource_a0.compiler_get_source_dir();
        char *input_path = ct_path_a0.join(a, 2, source_dir, input_str);

        uint32_t postprocess_flag = aiProcessPreset_TargetRealtime_MaxQuality;

        if (yaml_is_valid(postprocess_n)) {
            YAML_NODE_SCOPE(node, postprocess_n, "flip_uvs",
                            if (yaml_is_valid(node) &&
                                yaml_as_bool(node))
                                postprocess_flag |= aiProcess_FlipUVs;);
        }

        const struct aiScene *scene = aiImportFile(input_path,
                                                   postprocess_flag |
                                                   aiProcess_MakeLeftHanded);

        char tmp_buffer[1024] = {0};
        char tmp_buffer2[1024] = {0};
        uint32_t unique = 0;
        for (int i = 0; i < scene->mNumMeshes; ++i) {
            struct aiMesh *mesh = scene->mMeshes[i];

            if (mesh->mName.length == 0) {
                snprintf(tmp_buffer, CETECH_ARRAY_LEN(tmp_buffer), "geom_%d",
                         i);
            } else {
                memcpy(tmp_buffer, mesh->mName.data, mesh->mName.length);
            }

            uint64_t name_id = ct_hash_a0.id64_from_str(tmp_buffer);
            for (int k = 0; k < array::size(output->geom_name); ++k) {
                if (name_id == output->geom_name[k]) {
                    snprintf(tmp_buffer2, CETECH_ARRAY_LEN(tmp_buffer2), "%s%d",
                             tmp_buffer, ++unique);
                    snprintf(tmp_buffer, CETECH_ARRAY_LEN(tmp_buffer), "%s",
                             tmp_buffer2);
                    break;
                }
            }

            array::push_back(output->geom_name,
                             ct_hash_a0.id64_from_str(tmp_buffer));
            array::push_back<uint64_t>(output->geom_node, 0);
            array::push_back(output->ib_offset, array::size(output->ib));
            array::push_back(output->vb_offset, array::size(output->vb));
            array::push_back(output->ib_size, mesh->mNumFaces * 3);

            bgfx::VertexDecl vertex_decl;
            vertex_decl.begin();

            uint32_t v_size = 0;
            if (mesh->mVertices != NULL) {
                vertex_decl.add(bgfx::Attrib::Position, 3,
                                bgfx::AttribType::Float, 0, 0);
                v_size += 3 * sizeof(float);
            }

            if (mesh->mNormals != NULL) {
                vertex_decl.add(bgfx::Attrib::Normal, 3,
                                bgfx::AttribType::Float, 0, 0);
                v_size += 3 * sizeof(float);
            }

            if (mesh->mTextureCoords[0] != NULL) {
                vertex_decl.add(bgfx::Attrib::TexCoord0, 2,
                                bgfx::AttribType::Float, 0, 0);
                v_size += 2 * sizeof(float);
            }
            vertex_decl.end();

            array::push_back(output->vb_decl, vertex_decl);
            array::push_back(output->vb_size, v_size * mesh->mNumVertices);

            for (int j = 0; j < mesh->mNumVertices; ++j) {
                if (mesh->mVertices != NULL) {
                    array::push(output->vb, (uint8_t *) &mesh->mVertices[j],
                                sizeof(float) * 3);
                }

                if (mesh->mNormals != NULL) {
                    array::push(output->vb, (uint8_t *) &mesh->mNormals[j],
                                sizeof(float) * 3);
                }

                if (mesh->mTextureCoords[0] != NULL) {
                    array::push(output->vb,
                                (uint8_t *) &mesh->mTextureCoords[0][j],
                                sizeof(float) * 2);
                }
            }

            for (int j = 0; j < mesh->mNumFaces; ++j) {
                array::push_back(output->ib, mesh->mFaces[j].mIndices[0]);
                array::push_back(output->ib, mesh->mFaces[j].mIndices[1]);
                array::push_back(output->ib, mesh->mFaces[j].mIndices[2]);
            }
        }

        _compile_assimp_node(scene->mRootNode, UINT32_MAX, output);

        yaml_node_free(import_n);
        return 1;
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

        struct compile_output *output = _crete_compile_output();

        int ret = 1;
        yaml_node_t import_node = yaml_get_node(root, "import");
        if (yaml_is_valid(import_node)) {
            ret = _compile_assimp(filename, root, output, compilator_api);
        } else {
            ret = _compile_yaml(root, output);
        }

        if (!ret) {
            _destroy_compile_output(output);
            CEL_FREE(ct_memory_a0.main_allocator(), source_data);
            return 0;
        }

        scene_blob::blob_t res = {
                .geom_count = (uint32_t) array::size(output->geom_name),
                .node_count = (uint32_t) array::size(output->node_name),
                .ib_len = (uint32_t) array::size(output->ib),
                .vb_len = (uint32_t) array::size(output->vb),
        };

        build_vio->write(build_vio->inst, &res, sizeof(res), 1);
        build_vio->write(build_vio->inst, array::begin(output->geom_name),
                         sizeof(uint64_t),
                         array::size(output->geom_name));
        build_vio->write(build_vio->inst, array::begin(output->ib_offset),
                         sizeof(uint32_t),
                         array::size(output->ib_offset));
        build_vio->write(build_vio->inst, array::begin(output->vb_offset),
                         sizeof(uint32_t),
                         array::size(output->vb_offset));
        build_vio->write(build_vio->inst, array::begin(output->vb_decl),
                         sizeof(bgfx::VertexDecl),
                         array::size(output->vb_decl));
        build_vio->write(build_vio->inst, array::begin(output->ib_size),
                         sizeof(uint32_t),
                         array::size(output->ib_size));
        build_vio->write(build_vio->inst, array::begin(output->vb_size),
                         sizeof(uint32_t),
                         array::size(output->vb_size));
        build_vio->write(build_vio->inst, array::begin(output->ib),
                         sizeof(uint32_t),
                         array::size(output->ib));
        build_vio->write(build_vio->inst, array::begin(output->vb),
                         sizeof(uint8_t),
                         array::size(output->vb));
        build_vio->write(build_vio->inst, array::begin(output->node_name),
                         sizeof(uint64_t),
                         array::size(output->node_name));
        build_vio->write(build_vio->inst, array::begin(output->node_parent),
                         sizeof(uint32_t),
                         array::size(output->node_parent));
        build_vio->write(build_vio->inst, array::begin(output->node_pose),
                         sizeof(float),
                         array::size(output->node_pose));
        build_vio->write(build_vio->inst, array::begin(output->geom_node),
                         sizeof(uint64_t),
                         array::size(output->geom_name));

        _destroy_compile_output(output);
        CEL_FREE(ct_memory_a0.main_allocator(), source_data);
        return 1;
    }

    int init(ct_api_a0 *api) {
        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_resource_a0);
        CETECH_GET_API(api, ct_scenegprah_a0);
        CETECH_GET_API(api, ct_path_a0);
        CETECH_GET_API(api, ct_vio_a0);
        CETECH_GET_API(api, ct_hash_a0);
        CETECH_GET_API(api, ct_thread_a0);

        ct_resource_a0.compiler_register(ct_hash_a0.id64_from_str("scene"),
                                         compiler);

        return 1;
    }

}

#endif