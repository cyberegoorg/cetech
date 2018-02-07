//==============================================================================
// Include
//==============================================================================

#include <include/assimp/scene.h>
#include <include/assimp/postprocess.h>
#include <include/assimp/cimport.h>
#include <cetech/macros.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/core/containers/array.h>

#include "cetech/core/containers/map.inl"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/engine/machine/machine.h"


#include "cetech/engine/resource/resource.h"
#include "cetech/engine/entity/entity.h"

#include "cetech/engine/scenegraph/scenegraph.h"

#include "scene_blob.h"
#include "cetech/core/os/path.h"
#include "cetech/core/os/thread.h"
#include "cetech/core/os/vio.h"

using namespace celib;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_scenegprah_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_thread_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);


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

typedef char char_128[128];
struct compile_output {
    uint64_t *geom_name;
    uint32_t *ib_offset;
    uint32_t *vb_offset;
    bgfx::VertexDecl *vb_decl;
    uint32_t *ib_size;
    uint32_t *vb_size;
    uint32_t *ib;
    uint8_t *vb;
    uint64_t *node_name;
    uint32_t *node_parent;
    float *node_pose;
    uint64_t *geom_node;
    char_128 *geom_str; // TODO : SHIT
    char_128 *node_str; // TODO : SHIT
};

struct compile_output *_crete_compile_output() {
    ct_alloc *a = ct_memory_a0.main_allocator();
    struct compile_output *output =
            CT_ALLOC(a, struct compile_output,
                     sizeof(struct compile_output));
    *output = {0};

    return output;
}

static void _destroy_compile_output(struct compile_output *output) {
    ct_alloc *a = ct_memory_a0.main_allocator();

    ct_array_free(output->geom_name, a);
    ct_array_free(output->ib_offset, a);
    ct_array_free(output->vb_offset, a);
    ct_array_free(output->vb_decl, a);
    ct_array_free(output->ib_size, a);
    ct_array_free(output->vb_size, a);
    ct_array_free(output->ib, a);
    ct_array_free(output->vb, a);
    ct_array_free(output->node_name, a);
    ct_array_free(output->geom_node, a);
    ct_array_free(output->node_parent, a);
    ct_array_free(output->node_pose, a);
    ct_array_free(output->node_str, a);
    ct_array_free(output->geom_str, a);

    CT_FREE(a, output);
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

static void _parse_vertex_decl(bgfx::VertexDecl *decl,
                               uint32_t *vertex_size,
                               bgfx::Attrib::Enum type,
                               ct_yamlng_node decl_node) {

    ct_yng_doc *d = decl_node.d;

    uint64_t keys[] = {
            d->hash(d, decl_node),
            ct_yng_a0.key("type"),
    };
    const char *type_str = d->get_str(d,
                                      ct_yng_a0.combine_key(keys,
                                                            CETECH_ARRAY_LEN(
                                                                    keys)),
                                      "");

    keys[1] = ct_yng_a0.key("size");
    float size = d->get_float(d, ct_yng_a0.combine_key(keys,
                                                       CETECH_ARRAY_LEN(
                                                               keys)),
                              0.0f);
    bgfx::AttribType::Enum attrib_type;
    size_t v_size;

    _type_to_attr_type(type_str, &attrib_type, &v_size);

    *vertex_size += int(size) * v_size;

    decl->add(type, (uint8_t) int(size), attrib_type, 0, 0);
}


static void _parese_types(bgfx::VertexDecl *decl,
                          ct_yamlng_node types,
                          uint32_t *vertex_size) {
    ct_yng_doc *d = types.d;

    for (uint32_t i = 0; i < CETECH_ARRAY_LEN(_chanel_types); ++i) {

        uint64_t keys[] = {
                d->hash(d, types),
                ct_yng_a0.key(_chanel_types[i].name),
        };

        ct_yamlng_node node = d->get(d,
                                     ct_yng_a0.combine_key(keys,
                                                           CETECH_ARRAY_LEN(
                                                                   keys)));
        if (0 != node.idx) {
            _parse_vertex_decl(decl, vertex_size,
                               _chanel_types[i].attrib,
                               node);
        }
    }
}

static void _write_chanel(struct ct_yamlng_node node,
                          struct ct_yamlng_node types,
                          size_t i,
                          const char *name,
                          struct ct_yamlng_node chanels_n,
                          struct compile_output *output) {
    struct ct_alloc *a = ct_memory_a0.main_allocator();

    bgfx::AttribType::Enum attrib_type;
    size_t v_size;

    ct_yng_doc *d = node.d;

    struct ct_yamlng_node idx_n = d->get_seq(d,
                                             d->hash(d, node), i);

    uint32_t idx = (uint32_t) (d->as_float(d, idx_n, 0.0f));

    uint32_t size = 0;
    const char *type_str = NULL;
    {
        uint64_t keys[] = {
                d->hash(d, types),
                ct_yng_a0.key(name),
                ct_yng_a0.key("type"),
        };
        type_str = d->get_str(d, ct_yng_a0.combine_key(keys,
                                                       CETECH_ARRAY_LEN(
                                                               keys)),
                              "");

        keys[2] = ct_yng_a0.key("size"),
                size = (uint32_t) d->get_float(d,
                                               ct_yng_a0.combine_key(
                                                       keys,
                                                       CETECH_ARRAY_LEN(
                                                               keys)),
                                               0.0f);
    }

    _type_to_attr_type(type_str, &attrib_type, &v_size);


    uint64_t keys[] = {
            d->hash(d, chanels_n),
            ct_yng_a0.key(name),
    };
    uint64_t chanel_data_n = ct_yng_a0.combine_key(keys,
                                                   CETECH_ARRAY_LEN(
                                                           keys));

    for (uint32_t k = 0; k < size; ++k) {
        struct ct_yamlng_node n = d->get_seq(d, chanel_data_n,
                                             (idx * size) + k);

        // TODO: type
        float v = d->as_float(d, n, 0.0f);
        //log_debug("casdsadsa", "%s:%d -  %f", name, k, v);

        ct_array_push_n(output->vb, (uint8_t *) &v, sizeof(v), a);
    }
}

static void foreach_geometries_clb(struct ct_yamlng_node key,
                                   struct ct_yamlng_node value,
                                   void *_data) {
    struct ct_alloc *a = ct_memory_a0.main_allocator();

    struct compile_output *output = (compile_output *) _data;
    ct_yng_doc *d = key.d;

    const char *name_str = d->as_string(d, key, "");
    uint64_t name = CT_ID64_0(name_str);

    char tmp_name[128];
    strncpy(tmp_name, name_str, 127);

    ct_array_push(output->geom_name, name, a);
    ct_array_push_n(output->geom_str, &tmp_name, 1, a);
    ct_array_push(output->geom_node, 0, a);
    ct_array_push(output->ib_offset, ct_array_size(output->ib), a);
    ct_array_push(output->vb_offset, ct_array_size(output->vb), a);

    // DECL
    bgfx::VertexDecl vertex_decl;
    vertex_decl.begin();

    uint64_t keys[] = {
            d->hash(d, value),
            ct_yng_a0.key("types"),
    };
    ct_yamlng_node types = d->get(d, ct_yng_a0.combine_key(keys,
                                                           CETECH_ARRAY_LEN(
                                                                   keys)));

    uint32_t vertex_size = 0;
    _parese_types(&vertex_decl, types, &vertex_size);

    vertex_decl.end();
    ct_array_push(output->vb_decl, vertex_decl, a);

    // IB, VB
    keys[1] = ct_yng_a0.key("chanels");
    ct_yamlng_node chanels_n = d->get(d,
                                      ct_yng_a0.combine_key(keys,
                                                            CETECH_ARRAY_LEN(
                                                                    keys)));


    keys[1] = ct_yng_a0.key("indices");
    uint64_t k = ct_yng_a0.combine_key(keys,
                                       CETECH_ARRAY_LEN(
                                               keys));
    ct_yamlng_node indices_n = d->get(d, k);

    keys[0] = k;
    keys[1] = ct_yng_a0.key("size");
    ct_yamlng_node i_size = d->get(d, ct_yng_a0.combine_key(keys,
                                                            CETECH_ARRAY_LEN(
                                                                    keys)));

    uint32_t vertex_count = (uint32_t) d->as_float(d, i_size, 0.0f);

    ct_array_push(output->ib_size, vertex_count, a);
    ct_array_push(output->vb_size, vertex_size * vertex_count, a);

    for (uint32_t i = 0; i < vertex_count; ++i) {
        for (uint32_t j = 0; j < CETECH_ARRAY_LEN(_chanel_types); ++j) {
            const char *name = _chanel_types[j].name;

            uint64_t keys[] = {
                    d->hash(d, indices_n),
                    ct_yng_a0.key(name),
            };

            ct_yamlng_node node = d->get(d,
                                         ct_yng_a0.combine_key(keys,
                                                               CETECH_ARRAY_LEN(
                                                                       keys)));
            if (0 != node.idx) {
                _write_chanel(node, types, i, name, chanels_n, output);
            }
        }

        ct_array_push(output->ib, i, a);
    }
}


struct foreach_graph_data {
    struct compile_output *output;
    uint32_t parent_idx;
};

static void foreach_graph_clb(struct ct_yamlng_node key,
                              struct ct_yamlng_node value,
                              void *_data) {
    struct ct_alloc *a = ct_memory_a0.main_allocator();

    struct foreach_graph_data *output = (foreach_graph_data *) _data;
    ct_yng_doc *d = key.d;

    const char *key_str = d->as_string(d, key, "");
    uint64_t node_name = CT_ID64_0(key_str);


    char tmp_name[128];
    strncpy(tmp_name, key_str, 127);
    ct_array_push_n(output->output->node_str, &tmp_name, 1, a);

    uint64_t keys[] = {
            d->hash(d, value),
            ct_yng_a0.key("local"),
    };
    ct_yamlng_node local_pose = d->get(d,
                                       ct_yng_a0.combine_key(keys,
                                                             CETECH_ARRAY_LEN(
                                                                     keys)));

    float pose[16];
    d->as_mat4(d, local_pose, pose);

    uint32_t idx = (uint32_t) ct_array_size(output->output->node_name);

    ct_array_push(output->output->node_name, node_name, a);
    ct_array_push(output->output->node_parent, output->parent_idx, a);
    ct_array_push_n(output->output->node_pose, pose, 16, a);


    keys[1] = ct_yng_a0.key("geometries");
    uint64_t geometries_k = ct_yng_a0.combine_key(keys,
                                                  CETECH_ARRAY_LEN(
                                                          keys));

    ct_yamlng_node geometries_n = d->get(d, geometries_k);
    if (0 != geometries_n.idx) {
        const size_t name_count = d->size(d, geometries_n);

        for (uint32_t i = 0; i < name_count; ++i) {
            struct ct_yamlng_node name_node = d->get_seq(d,
                                                         geometries_k, i);
            const char *geom_str = d->as_string(d, name_node, "");

            uint64_t geom_name = CT_ID64_0(geom_str);
            for (uint32_t j = 0;
                 j < ct_array_size(output->output->geom_name); ++j) {
                if (geom_name != output->output->geom_name[j]) {
                    continue;
                }

                output->output->geom_node[j] = node_name;
                break;
            }

        }
    }


    keys[1] = ct_yng_a0.key("children");
    uint64_t children_k = ct_yng_a0.combine_key(keys,
                                                CETECH_ARRAY_LEN(
                                                        keys));
    ct_yamlng_node children_n = d->get(d,
                                       children_k);

    if (idx != children_n.idx) {
        struct foreach_graph_data graph_data = {
                .parent_idx = idx,
                .output = output->output
        };

        d->foreach_dict_node(d, children_n, foreach_graph_clb,
                             &graph_data);
    }
}

static int _compile_yaml(struct ct_yng_doc *document,
                         struct compile_output *output) {

    ct_yamlng_node geometries = document->get(document,
                                              ct_yng_a0.key("geometries"));
    ct_yamlng_node graph = document->get(document,
                                         ct_yng_a0.key("graph"));

    document->foreach_dict_node(document, geometries,
                                foreach_geometries_clb, output);

    struct foreach_graph_data graph_data = {
            .parent_idx = UINT32_MAX,
            .output = output
    };

    document->foreach_dict_node(document, graph,
                                foreach_graph_clb, &graph_data);
    return 1;
}

static void _compile_assimp_node(struct aiNode *root,
                                 uint32_t parent,
                                 struct compile_output *output) {
    struct ct_alloc *a = ct_memory_a0.main_allocator();

    uint64_t name = CT_ID64_0(root->mName.data);

    char tmp_name[128] = {0};
    strncpy(tmp_name, root->mName.data, 127);
    ct_array_push_n(output->node_str, &tmp_name, 1, a);

    uint32_t idx = ct_array_size(output->node_name);

    ct_array_push(output->node_name, name, a);
    ct_array_push(output->node_parent, parent, a);
    ct_array_push_n(output->node_pose, &root->mTransformation.a1, 16, a);

    for (uint32_t i = 0; i < root->mNumChildren; ++i) {
        _compile_assimp_node(root->mChildren[i], idx, output);
    }

    for (uint32_t i = 0; i < root->mNumMeshes; ++i) {
        ct_array_push(output->geom_node, name, a);
    }
}

static int _compile_assimp(const char *filename,
                           struct ct_yng_doc *doc,
                           struct compile_output *output,
                           ct_compilator_api *capi) {
    auto a = ct_memory_a0.main_allocator();

    const char *input_str = doc->get_str(doc,
                                         ct_yng_a0.key("import.input"), "");

    capi->add_dependency(filename, input_str);

    const char *source_dir = ct_resource_a0.compiler_get_source_dir();
    char *input_path = NULL;
    ct_path_a0.join(&input_path, a, 2, source_dir, input_str);

    uint32_t postprocess_flag = aiProcessPreset_TargetRealtime_MaxQuality;

    if (doc->get_bool(doc,
                      ct_yng_a0.key(
                              "import.postprocess.flip_uvs"),
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
            snprintf(tmp_buffer, CETECH_ARRAY_LEN(tmp_buffer), "geom_%d", i);
        } else {
            memcpy(tmp_buffer, mesh->mName.data, mesh->mName.length);
        }

        uint64_t name_id = CT_ID64_0(tmp_buffer);
        for (uint32_t k = 0; k < ct_array_size(output->geom_name); ++k) {
            if (name_id == output->geom_name[k]) {
                snprintf(tmp_buffer2, CETECH_ARRAY_LEN(tmp_buffer2), "%s%d",
                         tmp_buffer, ++unique);
                snprintf(tmp_buffer, CETECH_ARRAY_LEN(tmp_buffer), "%s",
                         tmp_buffer2);
                break;
            }
        }


        char tmp_name[128] = {0};
        strncpy(tmp_name, tmp_buffer, 127);
        ct_array_push_n(output->geom_str, &tmp_name, 1, a);

        ct_array_push(output->geom_name, CT_ID64_0(tmp_buffer), a);
        ct_array_push(output->ib_offset, ct_array_size(output->ib), a);
        ct_array_push(output->vb_offset, ct_array_size(output->vb), a);
        ct_array_push(output->ib_size, mesh->mNumFaces * 3, a);

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

        ct_array_push(output->vb_decl, vertex_decl, a);
        ct_array_push(output->vb_size, v_size * mesh->mNumVertices, a);

        for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
            if (mesh->mVertices != NULL) {
                ct_array_push_n(output->vb,
                                (uint8_t *) &mesh->mVertices[j],
                                sizeof(float) * 3, a);
            }

            if (mesh->mNormals != NULL) {
                ct_array_push_n(output->vb,
                                (uint8_t *) &mesh->mNormals[j],
                                sizeof(float) * 3, a);
            }

            if (mesh->mTextureCoords[0] != NULL) {
                ct_array_push_n(output->vb,
                                (uint8_t *) &mesh->mTextureCoords[0][j],
                                sizeof(float) * 2, a);
            }
        }

        for (uint32_t j = 0; j < mesh->mNumFaces; ++j) {
            ct_array_push(output->ib, mesh->mFaces[j].mIndices[0], a);
            ct_array_push(output->ib, mesh->mFaces[j].mIndices[1], a);
            ct_array_push(output->ib, mesh->mFaces[j].mIndices[2], a);
        }
    }

    _compile_assimp_node(scene->mRootNode, UINT32_MAX, output);
    return 1;
}

static void compiler(const char *filename,
                     char **output_blob,
                     struct ct_compilator_api *compilator_api) {

    struct compile_output *output = _crete_compile_output();
    struct ct_alloc *a = ct_memory_a0.main_allocator();

    ct_yng_doc *document = ct_ydb_a0.get(filename);

    int ret = 1;

    if (document->has_key(document,
                          ct_yng_a0.key("import"))) {
        ret = _compile_assimp(filename, document, output, compilator_api);
    } else {
        ret = _compile_yaml(document, output);
    }

    if (!ret) {
        _destroy_compile_output(output);
        return;
    }

    scene_blob::blob_t res = {
            .geom_count = (uint32_t) ct_array_size(output->geom_name),
            .node_count = (uint32_t) ct_array_size(output->node_name),
            .ib_len = (uint32_t) ct_array_size(output->ib),
            .vb_len = (uint32_t) ct_array_size(output->vb),
    };

    ct_array_push_n(*output_blob, &res, sizeof(res), a);
    ct_array_push_n(*output_blob, output->geom_name,
                    sizeof(uint64_t) *
                    ct_array_size(output->geom_name), a);
    ct_array_push_n(*output_blob, output->ib_offset,
                    sizeof(uint32_t) *
                    ct_array_size(output->ib_offset), a);
    ct_array_push_n(*output_blob, output->vb_offset,
                    sizeof(uint32_t) *
                    ct_array_size(output->vb_offset), a);
    ct_array_push_n(*output_blob, output->vb_decl,
                    sizeof(bgfx::VertexDecl) *
                    ct_array_size(output->vb_decl), a);
    ct_array_push_n(*output_blob, output->ib_size,
                    sizeof(uint32_t) *
                    ct_array_size(output->ib_size), a);
    ct_array_push_n(*output_blob, output->vb_size,
                    sizeof(uint32_t) *
                    ct_array_size(output->vb_size), a);
    ct_array_push_n(*output_blob, output->ib,
                    sizeof(uint32_t) *
                    ct_array_size(output->ib), a);
    ct_array_push_n(*output_blob, output->vb,
                    sizeof(uint8_t) *
                    ct_array_size(output->vb), a);
    ct_array_push_n(*output_blob, output->node_name,
                    sizeof(uint64_t) *
                    ct_array_size(output->node_name), a);

    ct_array_push_n(*output_blob, output->node_parent,
                    sizeof(uint32_t) *
                    ct_array_size(output->node_parent), a);
    ct_array_push_n(*output_blob, output->node_pose,
                    sizeof(float) *
                    ct_array_size(output->node_pose), a);
    ct_array_push_n(*output_blob, output->geom_node,
                    sizeof(uint64_t) *
                    ct_array_size(output->geom_name), a);

    ct_array_push_n(*output_blob, output->geom_str,
                    sizeof(char[128]) *
                    ct_array_size(output->geom_str), a);

    ct_array_push_n(*output_blob, output->node_str,
                    sizeof(char[128]) *
                    ct_array_size(output->node_str), a);

    _destroy_compile_output(output);
}

int scenecompiler_init(ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_scenegprah_a0);
    CETECH_GET_API(api, ct_path_a0);
    CETECH_GET_API(api, ct_vio_a0);
    CETECH_GET_API(api, ct_hashlib_a0);
    CETECH_GET_API(api, ct_thread_a0);
    CETECH_GET_API(api, ct_yng_a0);
    CETECH_GET_API(api, ct_ydb_a0);

    ct_resource_a0.compiler_register(CT_ID64_0("scene"),
                                     compiler, true);

    return 1;
}
