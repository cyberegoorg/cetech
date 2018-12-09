//==============================================================================
// Include
//==============================================================================
#include <time.h>

#include <celib/macros.h>
#include <celib/ydb.h>
#include <celib/array.inl>
#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include <celib/os.h>
#include <celib/ydb.h>
#include <celib/cdb.h>
#include <celib/config.h>


#include "cetech/machine/machine.h"
#include "cetech/resource/resource.h"
#include <cetech/renderer/renderer.h>
#include "cetech/ecs/ecs.h"
#include <cetech/scene/scene.h>
#include <cetech/kernel/kernel.h>
#include <cetech/resource/builddb.h>

#include <include/assimp/scene.h>
#include <include/assimp/postprocess.h>
#include <include/assimp/cimport.h>
#include <celib/log.h>
#include <celib/buffer.inl>
#include <cetech/resource/resource_compiler.h>


#define _G scene_compiler_globals

struct _G {
    struct ce_alloc *allocator;
} _G;

typedef char char_128[128];

struct compile_output {
    uint64_t *geom_name;
    uint32_t *ib_offset;
    uint32_t *vb_offset;
    ct_render_vertex_decl_t *vb_decl;
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

//static const struct {
//    const char *name;
//    ct_render_attrib_t attrib;
//} _chanel_types[] = {
//        {.name="position", .attrib=CT_RENDER_ATTRIB_POSITION},
//        {.name="normal", .attrib=CT_RENDER_ATTRIB_NORMAL},
//        {.name="tangent", .attrib=CT_RENDER_ATTRIB_TANGENT},
//        {.name="bitangent", .attrib=CT_RENDER_ATTRIB_BITANGENT},
//        {.name="color0", .attrib=CT_RENDER_ATTRIB_COLOR0},
//        {.name="color1", .attrib=CT_RENDER_ATTRIB_COLOR1},
//        {.name="color2", .attrib=CT_RENDER_ATTRIB_COLOR2},
//        {.name="color3", .attrib=CT_RENDER_ATTRIB_COLOR3},
//        {.name="indices", .attrib=CT_RENDER_ATTRIB_INDICES},
//        {.name="weight", .attrib=CT_RENDER_ATTRIB_INDICES},
//        {.name="texcoord0", .attrib=CT_RENDER_ATTRIB_TEXCOORD0},
//        {.name="texcoord1", .attrib=CT_RENDER_ATTRIB_TEXCOORD1},
//        {.name="texcoord2", .attrib=CT_RENDER_ATTRIB_TEXCOORD2},
//        {.name="texcoord3", .attrib=CT_RENDER_ATTRIB_TEXCOORD3},
//        {.name="texcoord4", .attrib=CT_RENDER_ATTRIB_TEXCOORD4},
//        {.name="texcoord5", .attrib=CT_RENDER_ATTRIB_TEXCOORD5},
//        {.name="texcoord6", .attrib=CT_RENDER_ATTRIB_TEXCOORD6},
//        {.name="texcoord7", .attrib=CT_RENDER_ATTRIB_TEXCOORD7},
//};
//
//static const struct {
//    const char *name;
//    size_t size;
//    ct_render_attrib_type_t attrib_type;
//} _attrin_tbl[] = {
//        {.name="f32", .size=sizeof(float), .attrib_type=CT_RENDER_ATTRIB_TYPE_FLOAT},
//        {.name="int16_t", .size=sizeof(int16_t), .attrib_type=CT_RENDER_ATTRIB_TYPE_INT16},
//        {.name="uint8_t", .size=sizeof(uint8_t), .attrib_type=CT_RENDER_ATTRIB_TYPE_UINT8},
////        {.name="float16_t", .size=sizeof(float16_t), .attrib_type=CT_RENDER_ATTRIB_TYPE_HALF},
//        // TODO: {.name="u10", .size=sizeof(u10), .attrib_type=BGFX_ATTRIB_TYPE_UINT10},
//};
//

struct compile_output *_crete_compile_output() {
    struct compile_output *output =
            CE_ALLOC(_G.allocator, struct compile_output,
                     sizeof(struct compile_output));
    *output = {};

    return output;
}

static void _destroy_compile_output(struct compile_output *output) {
    ce_array_free(output->geom_name, _G.allocator);
    ce_array_free(output->ib_offset, _G.allocator);
    ce_array_free(output->vb_offset, _G.allocator);
    ce_array_free(output->vb_decl, _G.allocator);
    ce_array_free(output->ib_size, _G.allocator);
    ce_array_free(output->vb_size, _G.allocator);
    ce_array_free(output->ib, _G.allocator);
    ce_array_free(output->vb, _G.allocator);
    ce_array_free(output->node_name, _G.allocator);
    ce_array_free(output->geom_node, _G.allocator);
    ce_array_free(output->node_parent, _G.allocator);
    ce_array_free(output->node_pose, _G.allocator);
    ce_array_free(output->node_str, _G.allocator);
    ce_array_free(output->geom_str, _G.allocator);

    CE_FREE(_G.allocator, output);
}

//static void _type_to_attr_type(const char *name,
//                               ct_render_attrib_type_t *attr_type,
//                               size_t *size) {
//
//    for (uint32_t i = 0; i < CE_ARRAY_LEN(_attrin_tbl); ++i) {
//        if (strcmp(_attrin_tbl[i].name, name) != 0) {
//            continue;
//        }
//
//
//        *attr_type = _attrin_tbl[i].attrib_type;
//        *size = _attrin_tbl[i].size;
//        return;
//    }
//
//    *attr_type = CT_RENDER_ATTRIB_TYPE_COUNT;
//    *size = 0;
//}
//
//static void _parse_vertex_decl(ct_render_vertex_decl_t *decl,
//                               uint32_t *vertex_size,
//                               ct_render_attrib type,
//                               ce_yng_node decl_node) {
//
//    ce_yng_doc *d = decl_node.d;
//
//    uint64_t keys[] = {
//            d->hash(d, decl_node),
//            ce_ydb_a0->key("type"),
//    };
//    const char *type_str = d->get_str(d,
//                                      ce_ydb_a0->combine_key(keys,
//                                                             CE_ARRAY_LEN(
//                                                                     keys)),
//                                      "");
//
//    keys[1] = ce_ydb_a0->key("size");
//    float size = d->get_float(d, ce_ydb_a0->combine_key(keys,
//                                                        CE_ARRAY_LEN(
//                                                                keys)),
//                              0.0f);
//    ct_render_attrib_type_t attrib_type;
//    size_t v_size;
//
//    _type_to_attr_type(type_str, &attrib_type, &v_size);
//
//    *vertex_size += int(size) * v_size;
//
//    ct_renderer_a0->vertex_decl_add(decl, type, (uint8_t) int(size),
//                                    attrib_type, 0, 0);
//}
//

//static void _parese_types(ct_render_vertex_decl_t *decl,
//                          ce_yng_node types,
//                          uint32_t *vertex_size) {
//    ce_yng_doc *d = types.d;
//
//    for (uint32_t i = 0; i < CE_ARRAY_LEN(_chanel_types); ++i) {
//
//        uint64_t keys[] = {
//                d->hash(d, types),
//                ce_ydb_a0->key(_chanel_types[i].name),
//        };
//
//        ce_yng_node node = d->get(d,
//                                  ce_ydb_a0->combine_key(keys,
//                                                         CE_ARRAY_LEN(
//                                                                 keys)));
//        if (0 != node.idx) {
//            _parse_vertex_decl(decl, vertex_size,
//                               _chanel_types[i].attrib, node);
//        }
//    }
//}
//
//static void _write_chanel(struct ce_yng_node node,
//                          struct ce_yng_node types,
//                          size_t i,
//                          const char *name,
//                          struct ce_yng_node chanels_n,
//                          struct compile_output *output) {
//
//    ct_render_attrib_type_t attrib_type;
//    size_t v_size;
//
//    ce_yng_doc *d = node.d;
//
//    struct ce_yng_node idx_n = d->get_seq(d,
//                                          d->hash(d, node), i);
//
//    uint32_t idx = (uint32_t) (d->as_float(d, idx_n, 0.0f));
//
//    uint32_t size = 0;
//    const char *type_str = NULL;
//    {
//        uint64_t keys[] = {
//                d->hash(d, types),
//                ce_ydb_a0->key(name),
//                ce_ydb_a0->key("type"),
//        };
//        type_str = d->get_str(d,
//                              ce_ydb_a0->combine_key(keys, CE_ARRAY_LEN(keys)),
//                              "");
//
//        keys[2] = ce_ydb_a0->key("size"),
//                size = (uint32_t) d->get_float(d,
//                                               ce_ydb_a0->combine_key(
//                                                       keys,
//                                                       CE_ARRAY_LEN(
//                                                               keys)),
//                                               0.0f);
//    }
//
//    _type_to_attr_type(type_str, &attrib_type, &v_size);
//
//
//    uint64_t keys[] = {
//            d->hash(d, chanels_n),
//            ce_ydb_a0->key(name),
//    };
//    uint64_t chanel_data_n = ce_ydb_a0->combine_key(keys,
//                                                    CE_ARRAY_LEN(
//                                                            keys));
//
//    for (uint32_t k = 0; k < size; ++k) {
//        struct ce_yng_node n = d->get_seq(d, chanel_data_n,
//                                          (idx * size) + k);
//        // TODO: ptype
//        float v = d->as_float(d, n, 0.0f);
//
//        ce_array_push_n(output->vb, (uint8_t *) &v, sizeof(v), _G.allocator);
//    }
//}
//
//static void foreach_geometries_clb(struct ce_yng_node key,
//                                   struct ce_yng_node value,
//                                   void *_data) {
//
//    struct compile_output *output = (compile_output *) _data;
//    ce_yng_doc *d = key.d;
//
//    const char *name_str = d->as_string(d, key, "");
//    uint64_t name = ce_id_a0->id64(name_str);
//
//    char tmp_name[128];
//    strncpy(tmp_name, name_str, 127);
//
//    ce_array_push(output->geom_name, name, _G.allocator);
//    ce_array_push_n(output->geom_str, &tmp_name, 1, _G.allocator);
//    ce_array_push(output->geom_node, 0, _G.allocator);
//    ce_array_push(output->ib_offset, ce_array_size(output->ib), _G.allocator);
//    ce_array_push(output->vb_offset, ce_array_size(output->vb), _G.allocator);
//
//    // DECL
//    ct_render_vertex_decl_t vertex_decl;
//    ct_renderer_a0->vertex_decl_begin(&vertex_decl,
//                                      CT_RENDER_RENDERER_TYPE_COUNT);
//
//    uint64_t keys[] = {
//            d->hash(d, value),
//            ce_ydb_a0->key("types"),
//    };
//    ce_yng_node types = d->get(d, ce_ydb_a0->combine_key(keys,
//                                                         CE_ARRAY_LEN(
//                                                                 keys)));
//
//    uint32_t vertex_size = 0;
//    _parese_types(&vertex_decl, types, &vertex_size);
//
//    ct_renderer_a0->vertex_decl_end(&vertex_decl);
//    ce_array_push(output->vb_decl, vertex_decl, _G.allocator);
//
//    // IB, VB
//    keys[1] = ce_ydb_a0->key("chanels");
//    ce_yng_node chanels_n = d->get(d,
//                                   ce_ydb_a0->combine_key(keys,
//                                                          CE_ARRAY_LEN(
//                                                                  keys)));
//
//
//    keys[1] = ce_ydb_a0->key("indices");
//    uint64_t k = ce_ydb_a0->combine_key(keys,
//                                        CE_ARRAY_LEN(
//                                                keys));
//    ce_yng_node indices_n = d->get(d, k);
//
//    keys[0] = k;
//    keys[1] = ce_ydb_a0->key("size");
//    ce_yng_node i_size = d->get(d, ce_ydb_a0->combine_key(keys,
//                                                          CE_ARRAY_LEN(
//                                                                  keys)));
//
//    uint32_t vertex_count = (uint32_t) d->as_float(d, i_size, 0.0f);
//
//    ce_array_push(output->ib_size, vertex_count, _G.allocator);
//    ce_array_push(output->vb_size, vertex_size * vertex_count, _G.allocator);
//
//    for (uint32_t i = 0; i < vertex_count; ++i) {
//        for (uint32_t j = 0; j < CE_ARRAY_LEN(_chanel_types); ++j) {
//            const char *name = _chanel_types[j].name;
//
//            uint64_t keys[] = {
//                    d->hash(d, indices_n),
//                    ce_ydb_a0->key(name),
//            };
//
//            ce_yng_node node = d->get(d,
//                                      ce_ydb_a0->combine_key(keys,
//                                                             CE_ARRAY_LEN(
//                                                                     keys)));
//            if (0 != node.idx) {
//                _write_chanel(node, types, i, name, chanels_n, output);
//            }
//        }
//
//        ce_array_push(output->ib, i, _G.allocator);
//    }
//}
//
//
//struct foreach_graph_data {
//    struct compile_output *output;
//    uint32_t parent_idx;
//};
//
//static void foreach_graph_clb(struct ce_yng_node key,
//                              struct ce_yng_node value,
//                              void *_data) {
//
//    struct foreach_graph_data *output = (foreach_graph_data *) _data;
//    ce_yng_doc *d = key.d;
//
//    const char *key_str = d->as_string(d, key, "");
//    uint64_t node_name = ce_id_a0->id64(key_str);
//
//
//    char tmp_name[128];
//    strncpy(tmp_name, key_str, 127);
//    ce_array_push_n(output->output->node_str, &tmp_name, 1, _G.allocator);
//
//    uint64_t keys[] = {
//            d->hash(d, value),
//            ce_ydb_a0->key("local"),
//    };
//    ce_yng_node local_pose = d->get(d,
//                                    ce_ydb_a0->combine_key(keys,
//                                                           CE_ARRAY_LEN(
//                                                                   keys)));
//
//    float pose[16];
//    d->as_mat4(d, local_pose, pose);
//
//    uint32_t idx = (uint32_t) ce_array_size(output->output->node_name);
//
//    ce_array_push(output->output->node_name, node_name, _G.allocator);
//    ce_array_push(output->output->node_parent, output->parent_idx,
//                  _G.allocator);
//    ce_array_push_n(output->output->node_pose, pose, 16, _G.allocator);
//
//
//    keys[1] = ce_ydb_a0->key("geometries");
//    uint64_t geometries_k = ce_ydb_a0->combine_key(keys,
//                                                   CE_ARRAY_LEN(
//                                                           keys));
//
//    ce_yng_node geometries_n = d->get(d, geometries_k);
//    if (0 != geometries_n.idx) {
//        const size_t name_count = d->size(d, geometries_n);
//
//        for (uint32_t i = 0; i < name_count; ++i) {
//            struct ce_yng_node name_node = d->get_seq(d,
//                                                      geometries_k, i);
//            const char *geom_str = d->as_string(d, name_node, "");
//
//            uint64_t geom_name = ce_id_a0->id64(geom_str);
//            for (uint32_t j = 0;
//                 j < ce_array_size(output->output->geom_name); ++j) {
//                if (geom_name != output->output->geom_name[j]) {
//                    continue;
//                }
//
//                output->output->geom_node[j] = node_name;
//                break;
//            }
//
//        }
//    }
//
//
//    keys[1] = ce_ydb_a0->key("children");
//    uint64_t children_k = ce_ydb_a0->combine_key(keys,
//                                                 CE_ARRAY_LEN(
//                                                         keys));
//    ce_yng_node children_n = d->get(d,
//                                    children_k);
//
//    if (idx != children_n.idx) {
//        struct foreach_graph_data graph_data = {
//                .parent_idx = idx,
//                .output = output->output
//        };
//
//        d->foreach_dict_node(d, children_n, foreach_graph_clb,
//                             &graph_data);
//    }
//}

//static int _compile_yaml(struct ce_yng_doc *document,
//                         struct compile_output *output) {
//
//    ce_yng_node geometries = document->get(document,
//                                           ce_ydb_a0->key("geometries"));
//    ce_yng_node graph = document->get(document,
//                                      ce_ydb_a0->key("graph"));
//
//    document->foreach_dict_node(document, geometries,
//                                foreach_geometries_clb, output);
//
//    struct foreach_graph_data graph_data = {
//            .parent_idx = UINT32_MAX,
//            .output = output
//    };
//
//    document->foreach_dict_node(document, graph,
//                                foreach_graph_clb, &graph_data);
//    return 1;
//}

static void _compile_assimp_node(struct aiNode *root,
                                 uint32_t parent,
                                 struct compile_output *output) {

    uint64_t name = ce_id_a0->id64(root->mName.data);

    char tmp_name[128] = {};
    strncpy(tmp_name, root->mName.data, 127);
    ce_array_push_n(output->node_str, &tmp_name, 1, _G.allocator);

    uint32_t idx = ce_array_size(output->node_name);

    ce_array_push(output->node_name, name, _G.allocator);
    ce_array_push(output->node_parent, parent, _G.allocator);
    ce_array_push_n(output->node_pose, &root->mTransformation.a1, 16,
                    _G.allocator);

    for (uint32_t i = 0; i < root->mNumChildren; ++i) {
        _compile_assimp_node(root->mChildren[i], idx, output);
    }

    for (uint32_t i = 0; i < root->mNumMeshes; ++i) {
        ce_array_push(output->geom_node, name, _G.allocator);
    }
}

static int _compile_assimp(uint64_t k,
                           struct compile_output *output) {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(k);

    uint64_t import_obj = ce_cdb_a0->read_subobject(reader, ce_id_a0->id64("import"),
                                                    0);

    const ce_cdb_obj_o *import_reader = ce_cdb_a0->read(import_obj);

    const char *input_str = ce_cdb_a0->read_str(import_reader,
                                                ce_id_a0->id64("input"), "");

    const ce_cdb_obj_o *c_reader = ce_cdb_a0->read(ce_config_a0->obj());
    const char *source_dir = ce_cdb_a0->read_str(c_reader,
                                                 CONFIG_SRC, "");
    char *input_path = NULL;
    ce_os_a0->path->join(&input_path, _G.allocator, 2, source_dir,
                         input_str);

    uint32_t postprocess_flag = aiProcessPreset_TargetRealtime_MaxQuality |
                                aiProcess_ConvertToLeftHanded;

    uint64_t postprocess_obj = ce_cdb_a0->read_subobject(import_reader,
                                                         ce_id_a0->id64(
                                                                 "postprocess"),
                                                         0);

    const ce_cdb_obj_o *pp_reader = ce_cdb_a0->read(postprocess_obj);

    if (ce_cdb_a0->read_bool(pp_reader, ce_id_a0->id64("flip_uvs"),
                             false)) {
        postprocess_flag |= aiProcess_FlipUVs;
    }

    const struct aiScene *scene = aiImportFile(input_path,
                                               postprocess_flag);

    if (!scene) {
        ce_log_a0->error("scene_compiler", "Could not import %s", input_path);
        return 0;
    }

    char tmp_buffer[1024] = {};
    char tmp_buffer2[1024] = {};
    uint32_t unique = 0;
    for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
        struct aiMesh *mesh = scene->mMeshes[i];

        if (mesh->mName.length == 0) {
            snprintf(tmp_buffer, CE_ARRAY_LEN(tmp_buffer), "geom_%d", i);
        } else {
            memcpy(tmp_buffer, mesh->mName.data, mesh->mName.length);
        }

        uint64_t name_id = ce_id_a0->id64(tmp_buffer);
        for (uint32_t k = 0; k < ce_array_size(output->geom_name); ++k) {
            if (name_id == output->geom_name[k]) {
                snprintf(tmp_buffer2, CE_ARRAY_LEN(tmp_buffer2), "%s%d",
                         tmp_buffer, ++unique);
                snprintf(tmp_buffer, CE_ARRAY_LEN(tmp_buffer), "%s",
                         tmp_buffer2);
                break;
            }
        }


        char tmp_name[128] = {};
        strncpy(tmp_name, tmp_buffer, 127);
        ce_array_push_n(output->geom_str, &tmp_name, 1, _G.allocator);

        ce_array_push(output->geom_name, ce_id_a0->id64(tmp_buffer),
                      _G.allocator);
        ce_array_push(output->ib_offset, ce_array_size(output->ib),
                      _G.allocator);
        ce_array_push(output->vb_offset, ce_array_size(output->vb),
                      _G.allocator);
        ce_array_push(output->ib_size, mesh->mNumFaces * 3, _G.allocator);

        ct_render_vertex_decl_t vertex_decl;
        ct_renderer_a0->vertex_decl_begin(&vertex_decl,
                                          CT_RENDER_RENDERER_TYPE_COUNT);

        uint32_t v_size = 0;
        if (mesh->mVertices != NULL) {
            ct_renderer_a0->vertex_decl_add(&vertex_decl,
                                            CT_RENDER_ATTRIB_POSITION, 3,
                                            CT_RENDER_ATTRIB_TYPE_FLOAT, 0, 0);
            v_size += 3 * sizeof(float);
        }

        if (mesh->mNormals != NULL) {
            ct_renderer_a0->vertex_decl_add(&vertex_decl,
                                            CT_RENDER_ATTRIB_NORMAL, 3,
                                            CT_RENDER_ATTRIB_TYPE_FLOAT, true,
                                            0);
            v_size += 3 * sizeof(float);
        }

        if (mesh->mTextureCoords[0] != NULL) {
            ct_renderer_a0->vertex_decl_add(&vertex_decl,
                                            CT_RENDER_ATTRIB_TEXCOORD0, 2,
                                            CT_RENDER_ATTRIB_TYPE_FLOAT, 0, 0);

            v_size += 2 * sizeof(float);
        }
        ct_renderer_a0->vertex_decl_end(&vertex_decl);

        ce_array_push(output->vb_decl, vertex_decl, _G.allocator);
        ce_array_push(output->vb_size, v_size * mesh->mNumVertices,
                      _G.allocator);

        for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
            if (mesh->mVertices != NULL) {
                ce_array_push_n(output->vb,
                                (uint8_t *) &mesh->mVertices[j],
                                sizeof(float) * 3, _G.allocator);
            }

            if (mesh->mNormals != NULL) {
                ce_array_push_n(output->vb,
                                (uint8_t *) &mesh->mNormals[j],
                                sizeof(float) * 3, _G.allocator);
            }

            if (mesh->mTextureCoords[0] != NULL) {
                ce_array_push_n(output->vb,
                                (uint8_t *) &mesh->mTextureCoords[0][j],
                                sizeof(float) * 2, _G.allocator);
            }
        }

        for (uint32_t j = 0; j < mesh->mNumFaces; ++j) {
            ce_array_push(output->ib, mesh->mFaces[j].mIndices[0],
                          _G.allocator);
            ce_array_push(output->ib, mesh->mFaces[j].mIndices[1],
                          _G.allocator);
            ce_array_push(output->ib, mesh->mFaces[j].mIndices[2],
                          _G.allocator);
        }
    }

    _compile_assimp_node(scene->mRootNode, UINT32_MAX, output);
    return 1;
}

extern "C" uint64_t scene_compiler(uint64_t k,
                                   struct ct_resource_id rid,
                                   const char *fullname) {
    struct compile_output *output = _crete_compile_output();

    int ret = 1;

    if (ce_cdb_a0->prop_exist(k, ce_id_a0->id64("import"))) {
        ret = _compile_assimp(k, output);
    }

    if (!ret) {
        _destroy_compile_output(output);
        return false;
    }

    uint64_t obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), SCENE_TYPE);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
    ce_cdb_a0->set_uint64(w, SCENE_GEOM_COUNT,
                          ce_array_size(output->geom_name));
    ce_cdb_a0->set_uint64(w, SCENE_NODE_COUNT,
                          ce_array_size(output->node_name));
    ce_cdb_a0->set_uint64(w, SCENE_IB_LEN, ce_array_size(output->ib));
    ce_cdb_a0->set_uint64(w, SCENE_VB_LEN, ce_array_size(output->vb));
    ce_cdb_a0->set_blob(w, SCENE_GEOM_NAME, output->geom_name,
                        sizeof(*output->geom_name) *
                        ce_array_size(output->geom_name));

    ce_cdb_a0->set_blob(w, SCENE_IB_OFFSET, output->ib_offset,
                        sizeof(*output->ib_offset) *
                        ce_array_size(output->ib_offset));
    ce_cdb_a0->set_blob(w, SCENE_VB_OFFSET, output->vb_offset,
                        sizeof(*output->vb_offset) *
                        ce_array_size(output->vb_offset));
    ce_cdb_a0->set_blob(w, SCENE_VB_DECL, output->vb_decl,
                        sizeof(*output->vb_decl) *
                        ce_array_size(output->vb_decl));
    ce_cdb_a0->set_blob(w, SCENE_IB_SIZE, output->ib_size,
                        sizeof(*output->ib_size) *
                        ce_array_size(output->ib_size));
    ce_cdb_a0->set_blob(w, SCENE_VB_SIZE, output->vb_size,
                        sizeof(*output->vb_size) *
                        ce_array_size(output->vb_size));
    ce_cdb_a0->set_blob(w, SCENE_IB_PROP, output->ib,
                        sizeof(*output->ib) * ce_array_size(output->ib));
    ce_cdb_a0->set_blob(w, SCENE_VB_PROP, output->vb,
                        sizeof(*output->vb) * ce_array_size(output->vb));
    ce_cdb_a0->set_blob(w, SCENE_NODE_NAME, output->node_name,
                        sizeof(*output->node_name) *
                        ce_array_size(output->node_name));
    ce_cdb_a0->set_blob(w, SCENE_NODE_PARENT, output->node_parent,
                        sizeof(*output->node_parent) *
                        ce_array_size(output->node_parent));
    ce_cdb_a0->set_blob(w, SCENE_NODE_POSE, output->node_pose,
                        sizeof(*output->node_pose) *
                        ce_array_size(output->node_pose));
    ce_cdb_a0->set_blob(w, SCENE_NODE_GEOM, output->geom_node,
                        sizeof(*output->geom_node) *
                        ce_array_size(output->geom_node));
    ce_cdb_a0->set_blob(w, SCENE_GEOM_STR, output->geom_str,
                        sizeof(*output->geom_str) *
                        ce_array_size(output->geom_str));
    ce_cdb_a0->set_blob(w, SCENE_NODE_STR, output->node_str,
                        sizeof(*output->node_str) *
                        ce_array_size(output->node_str));
    ce_cdb_a0->write_commit(w);


    _destroy_compile_output(output);

    return obj;
}

extern "C" int scenecompiler_init(struct ce_api_a0 *api) {
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_os_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ct_renderer_a0);

    _G = (struct _G) {.allocator=ce_memory_a0->system};


    return 1;
}
