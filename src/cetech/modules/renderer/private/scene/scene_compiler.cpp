//==============================================================================
// Include
//==============================================================================

#include <include/assimp/scene.h>
#include <include/assimp/postprocess.h>
#include <include/assimp/cimport.h>
#include <cetech/kernel/macros.h>

#include "celib/allocator.h"
#include "celib/map.inl"

#include "cetech/kernel/hashlib.h"
#include "cetech/kernel/memory.h"
#include "cetech/kernel/api_system.h"
#include "cetech/modules/machine/machine.h"


#include "cetech/kernel/resource.h"
#include "cetech/modules/entity/entity.h"

#include "cetech/modules/scenegraph/scenegraph.h"

#include "scene_blob.h"
#include "cetech/kernel/path.h"
#include "cetech/kernel/thread.h"
#include "cetech/kernel/vio.h"

using namespace celib;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_scenegprah_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_thread_a0);
CETECH_DECL_API(ct_yamlng_a0);


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

        for (uint32_t i = 0; i < CETECH_ARRAY_LEN(_attrin_tbl); ++i) {
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
                            ct_yamlng_node decl_node) {

        ct_yamlng_document *d = decl_node.d;

        uint64_t keys[] = {
                d->hash(d->inst, decl_node),
                ct_yamlng_a0.calc_key("type"),
        };
        const char *type_str = d->get_string(d->inst,
                                             ct_yamlng_a0.combine_key(keys,
                                                                      CETECH_ARRAY_LEN(
                                                                              keys)),
                                             "");

        keys[1] = ct_yamlng_a0.calc_key("size");
        float size = d->get_float(d->inst, ct_yamlng_a0.combine_key(keys,
                                                                    CETECH_ARRAY_LEN(
                                                                            keys)),
                                  0.0f);
        bgfx::AttribType::Enum attrib_type;
        size_t v_size;

        _type_to_attr_type(type_str, &attrib_type, &v_size);

        *vertex_size += int(size) * v_size;

        decl->add(type, (uint8_t) int(size),
                  attrib_type,
                  0, 0);
    }


    static void _parese_types(bgfx::VertexDecl *decl,
                              ct_yamlng_node types,
                              uint32_t *vertex_size) {
        ct_yamlng_document *d = types.d;

        for (uint32_t i = 0; i < CETECH_ARRAY_LEN(_chanel_types); ++i) {

            uint64_t keys[] = {
                    d->hash(d->inst, types),
                    ct_yamlng_a0.calc_key(_chanel_types[i].name),
            };

            ct_yamlng_node node = d->get(d->inst,
                                         ct_yamlng_a0.combine_key(keys,
                                                                  CETECH_ARRAY_LEN(
                                                                          keys)));
            if (0 != node.idx) {
                _parse_vertex_decl(decl, vertex_size,
                                   _chanel_types[i].attrib,
                                   node);
            }
        }
    }

    void _write_chanel(struct ct_yamlng_node node,
                       struct ct_yamlng_node types,
                       size_t i,
                       const char *name,
                       struct ct_yamlng_node chanels_n,
                       struct compile_output *output) {
        bgfx::AttribType::Enum attrib_type;
        size_t v_size;

        ct_yamlng_document *d = node.d;

        struct ct_yamlng_node idx_n = d->get_seq(d->inst,
                                                 d->hash(d->inst, node), i);

        uint32_t idx = (uint32_t) (d->as_float(d->inst, idx_n, 0.0f));

        uint32_t size = 0;
        const char *type_str = NULL;
        {
            uint64_t keys[] = {
                    d->hash(d->inst,types),
                    ct_yamlng_a0.calc_key(name),
                    ct_yamlng_a0.calc_key("type"),
            };
            type_str = d->get_string(d->inst, ct_yamlng_a0.combine_key(keys,
                                                                       CETECH_ARRAY_LEN(
                                                                               keys)),
                                     "");

            keys[2] = ct_yamlng_a0.calc_key("size"),
                    size = (uint32_t) d->get_float(d->inst,
                                                   ct_yamlng_a0.combine_key(
                                                           keys,
                                                           CETECH_ARRAY_LEN(
                                                                   keys)),
                                                   0.0f);
        }

        _type_to_attr_type(type_str, &attrib_type, &v_size);


        uint64_t keys[] = {
                d->hash(d->inst, chanels_n),
                ct_yamlng_a0.calc_key(name),
        };
        uint64_t chanel_data_n = ct_yamlng_a0.combine_key(keys,
                                                          CETECH_ARRAY_LEN(
                                                                  keys));

        for (uint32_t k = 0; k < size; ++k) {
            struct ct_yamlng_node n = d->get_seq(d->inst, chanel_data_n,
                                                 (idx * size) + k);

            // TODO: type
            float v = d->as_float(d->inst, n, 0.0f);
            //log_debug("casdsadsa", "%s:%d -  %f", name, k, v);

            array::push(output->vb, (uint8_t *) &v, sizeof(v));
        }
    }

    void foreach_geometries_clb(struct ct_yamlng_node key,
                                struct ct_yamlng_node value,
                                void *_data) {

        struct compile_output *output = (compile_output *) _data;
        ct_yamlng_document *d = key.d;

        const char *name_str = d->as_string(d->inst, key, "");
        uint64_t name = ct_hash_a0.id64_from_str(name_str);

        array::push_back(output->geom_name, name);
        array::push_back<uint64_t>(output->geom_node, 0);
        array::push_back(output->ib_offset, array::size(output->ib));
        array::push_back(output->vb_offset, array::size(output->vb));

        // DECL
        bgfx::VertexDecl vertex_decl;
        vertex_decl.begin();

        uint64_t keys[] = {
                d->hash(d->inst, value),
                ct_yamlng_a0.calc_key("types"),
        };
        ct_yamlng_node types = d->get(d->inst, ct_yamlng_a0.combine_key(keys,
                                                                        CETECH_ARRAY_LEN(
                                                                                keys)));

        uint32_t vertex_size = 0;
        _parese_types(&vertex_decl, types, &vertex_size);

        vertex_decl.end();
        array::push_back(output->vb_decl, vertex_decl);

        // IB, VB
        keys[1] = ct_yamlng_a0.calc_key("chanels");
        ct_yamlng_node chanels_n = d->get(d->inst,
                                          ct_yamlng_a0.combine_key(keys,
                                                                   CETECH_ARRAY_LEN(
                                                                           keys)));


        keys[1] = ct_yamlng_a0.calc_key("indices");
        uint64_t  k = ct_yamlng_a0.combine_key(keys,
                                               CETECH_ARRAY_LEN(
                                                       keys));
        ct_yamlng_node indices_n = d->get(d->inst, k);

        keys[0] = k;
        keys[1] = ct_yamlng_a0.calc_key("size");
        ct_yamlng_node i_size = d->get(d->inst, ct_yamlng_a0.combine_key(keys,
                                                                         CETECH_ARRAY_LEN(
                                                                                 keys)));

        uint32_t vertex_count = (uint32_t) d->as_float(d->inst, i_size, 0.0f);

        array::push_back(output->ib_size, vertex_count);
        array::push_back(output->vb_size, vertex_size * vertex_count);

        for (uint32_t i = 0; i < vertex_count; ++i) {
            for (uint32_t j = 0; j < CETECH_ARRAY_LEN(_chanel_types); ++j) {
                const char *name = _chanel_types[j].name;

                uint64_t keys[] = {
                        d->hash(d->inst, indices_n),
                        ct_yamlng_a0.calc_key(name),
                };

                ct_yamlng_node node = d->get(d->inst,
                                             ct_yamlng_a0.combine_key(keys,
                                                                      CETECH_ARRAY_LEN(
                                                                              keys)));
                if (0 != node.idx) {
                    _write_chanel(node, types, i, name, chanels_n, output);
                }
            }

            array::push_back(output->ib, (uint32_t) i);
        }
    }


    struct foreach_graph_data {
        struct compile_output *output;
        uint32_t parent_idx;
    };

    void foreach_graph_clb(struct ct_yamlng_node key,
                           struct ct_yamlng_node value,
                           void *_data) {
        struct foreach_graph_data *output = (foreach_graph_data *) _data;
        ct_yamlng_document *d = key.d;

        const char *key_str = d->as_string(d->inst, key, "");
        uint64_t node_name = ct_hash_a0.id64_from_str(key_str);

        uint64_t keys[] = {
                d->hash(d->inst, value),
                ct_yamlng_a0.calc_key("local"),
        };
        ct_yamlng_node local_pose = d->get(d->inst,
                                           ct_yamlng_a0.combine_key(keys,
                                                                    CETECH_ARRAY_LEN(
                                                                            keys)));

        float pose[16];
        d->as_mat4(d->inst, local_pose, pose);

        uint32_t idx = (uint32_t) array::size(output->output->node_name);

        array::push_back(output->output->node_name, node_name);
        array::push_back(output->output->node_parent, output->parent_idx);
        array::push(output->output->node_pose, pose, 16);


        keys[1] = ct_yamlng_a0.calc_key("geometries");
        uint64_t geometries_k = ct_yamlng_a0.combine_key(keys,
                                                         CETECH_ARRAY_LEN(
                                                                 keys));

        ct_yamlng_node geometries_n = d->get(d->inst, geometries_k);
        if (0 != geometries_n.idx) {
            const size_t name_count = d->size(d->inst, geometries_n);

            for (uint32_t i = 0; i < name_count; ++i) {
                struct ct_yamlng_node name_node = d->get_seq(d->inst,
                                                             geometries_k, i);
                const char *geom_str = d->as_string(d->inst, name_node, "");

                uint64_t geom_name = ct_hash_a0.id64_from_str(geom_str);
                for (uint32_t j = 0;
                     j < array::size(output->output->geom_name); ++j) {
                    if (geom_name != output->output->geom_name[j]) {
                        continue;
                    }

                    output->output->geom_node[j] = node_name;
                    break;
                }

            }
        }


        keys[1] = ct_yamlng_a0.calc_key("children");
        uint64_t children_k = ct_yamlng_a0.combine_key(keys,
                                                       CETECH_ARRAY_LEN(
                                                               keys));
        ct_yamlng_node children_n = d->get(d->inst,
                                           children_k);

        if (idx != children_n.idx) {
            struct foreach_graph_data graph_data = {
                    .parent_idx = idx,
                    .output = output->output
            };

            d->foreach_dict_node(d->inst, children_n, foreach_graph_clb,
                                 &graph_data);
        }
    }

    int _compile_yaml(struct ct_yamlng_document *document,
                      struct compile_output *output) {

        ct_yamlng_node geometries = document->get(document->inst,
                                                  ct_yamlng_a0.calc_key(
                                                          "geometries"));
        ct_yamlng_node graph = document->get(document->inst,
                                             ct_yamlng_a0.calc_key("graph"));

        document->foreach_dict_node(document->inst, geometries,
                                    foreach_geometries_clb, output);

        struct foreach_graph_data graph_data = {
                .parent_idx = UINT32_MAX,
                .output = output
        };

        document->foreach_dict_node(document->inst, graph,
                                    foreach_graph_clb, &graph_data);
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

        for (uint32_t i = 0; i < root->mNumChildren; ++i) {
            _compile_assimp_node(root->mChildren[i], idx, output);
        }

        for (uint32_t i = 0; i < root->mNumMeshes; ++i) {
            array::push_back(output->geom_node, name);
        }
    }

    int _compile_assimp(const char *filename,
                        struct ct_yamlng_document *document,
                        struct compile_output *output,
                        ct_compilator_api *capi) {
        auto a = ct_memory_a0.main_allocator();

        const char *input_str = document->get_string(
                document->inst,
                ct_yamlng_a0.calc_key("import.input"), "");

        capi->add_dependency(filename, input_str);

        const char *source_dir = ct_resource_a0.compiler_get_source_dir();
        char *input_path = ct_path_a0.join(a, 2, source_dir, input_str);

        uint32_t postprocess_flag = aiProcessPreset_TargetRealtime_MaxQuality;

        if (document->get_bool(document->inst,
                               ct_yamlng_a0.calc_key("import.postprocess.flip_uvs"),
                               false)) {
            postprocess_flag |= aiProcess_FlipUVs;
        }

        const struct aiScene *scene = aiImportFile(input_path,
                                                   postprocess_flag);

        char tmp_buffer[1024] = {};
        char tmp_buffer2[1024] = {};
        uint32_t unique = 0;
        for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
            struct aiMesh *mesh = scene->mMeshes[i];

            if (mesh->mName.length == 0) {
                snprintf(tmp_buffer, CETECH_ARRAY_LEN(tmp_buffer), "geom_%d",
                         i);
            } else {
                memcpy(tmp_buffer, mesh->mName.data, mesh->mName.length);
            }

            uint64_t name_id = ct_hash_a0.id64_from_str(tmp_buffer);
            for (uint32_t k = 0; k < array::size(output->geom_name); ++k) {
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

            for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
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

            for (uint32_t j = 0; j < mesh->mNumFaces; ++j) {
                array::push_back(output->ib, mesh->mFaces[j].mIndices[0]);
                array::push_back(output->ib, mesh->mFaces[j].mIndices[1]);
                array::push_back(output->ib, mesh->mFaces[j].mIndices[2]);
            }
        }

        _compile_assimp_node(scene->mRootNode, UINT32_MAX, output);
        return 1;
    }

    int compiler(const char *filename,
                 struct ct_yamlng_document *document,
                 ct_vio *build_vio,
                 ct_compilator_api *compilator_api) {

        struct compile_output *output = _crete_compile_output();

        int ret = 1;

        if (document->has_key(document->inst,
                              ct_yamlng_a0.calc_key("import"))) {
            ret = _compile_assimp(filename, document, output, compilator_api);
        } else {
            ret = _compile_yaml(document, output);
        }

        if (!ret) {
            _destroy_compile_output(output);
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
        CETECH_GET_API(api, ct_yamlng_a0);

        ct_resource_a0.compiler_register_yaml(ct_hash_a0.id64_from_str("scene"),
                                              compiler);

        return 1;
    }

}
