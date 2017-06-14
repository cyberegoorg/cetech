#ifndef CETECH_SCENE_COMPILER_H
#define CETECH_SCENE_COMPILER_H

#include <stdio.h>

#include <include/assimp/scene.h>
#include <include/assimp/postprocess.h>
#include <include/assimp/cimport.h>

#include <cetech/core/yaml.h>
#include <cetech/core/path.h>

static const struct {
    const char *name;
    bgfx_attrib_t attrib;
} _chanel_types[] = {
        {.name="position", .attrib=BGFX_ATTRIB_POSITION},
        {.name="normal", .attrib=BGFX_ATTRIB_NORMAL},
        {.name="tangent", .attrib=BGFX_ATTRIB_TANGENT},
        {.name="bitangent", .attrib=BGFX_ATTRIB_BITANGENT},
        {.name="color0", .attrib=BGFX_ATTRIB_COLOR0},
        {.name="color1", .attrib=BGFX_ATTRIB_COLOR1},
        {.name="indices", .attrib=BGFX_ATTRIB_INDICES},
        {.name="weight", .attrib=BGFX_ATTRIB_WEIGHT},
        {.name="texcoord0", .attrib=BGFX_ATTRIB_TEXCOORD0},
        {.name="texcoord1", .attrib=BGFX_ATTRIB_TEXCOORD1},
        {.name="texcoord2", .attrib=BGFX_ATTRIB_TEXCOORD2},
        {.name="texcoord3", .attrib=BGFX_ATTRIB_TEXCOORD3},
        {.name="texcoord4", .attrib=BGFX_ATTRIB_TEXCOORD4},
        {.name="texcoord5", .attrib=BGFX_ATTRIB_TEXCOORD5},
        {.name="texcoord6", .attrib=BGFX_ATTRIB_TEXCOORD6},
        {.name="texcoord7", .attrib=BGFX_ATTRIB_TEXCOORD7},
};

static const struct {
    const char *name;
    bgfx_attrib_type_t attrib_type;
    size_t size;
} _attrin_tbl[] = {
        {.name="f32", .size=sizeof(float), .attrib_type=BGFX_ATTRIB_TYPE_FLOAT},
        {.name="int16_t", .size=sizeof(int16_t), .attrib_type=BGFX_ATTRIB_TYPE_INT16},
        {.name="uint8_t", .size=sizeof(uint8_t), .attrib_type=BGFX_ATTRIB_TYPE_UINT8},
        // TODO: {.name="f16", .size=sizeof(f16), .attrib_type=BGFX_ATTRIB_TYPE_HALF},
        // TODO: {.name="u10", .size=sizeof(u10), .attrib_type=BGFX_ATTRIB_TYPE_UINT10},
};

struct compile_output {
    Array<uint64_t> geom_name;
    Array<uint32_t> ib_offset;
    Array<uint32_t> vb_offset;
    Array<bgfx_vertex_decl_t> vb_decl;
    Array<uint32_t> ib_size;
    Array<uint32_t> vb_size;
    Array<uint32_t> ib;
    Array<uint8_t> vb;

    Array<uint64_t> node_name;
    Array<uint32_t> node_parent;
    Array<mat44f_t> node_pose;
    Array<uint64_t> geom_node;
};

struct compile_output *_crete_compile_output() {
    struct allocator *a = memory_api_v0.main_allocator();
    struct compile_output *output =
    CETECH_ALLOCATE(a, struct compile_output, 1);

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
    struct allocator *a = memory_api_v0.main_allocator();

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

    CETECH_DEALLOCATE(a, output);
}

static void _type_to_attr_type(const char *name,
                               bgfx_attrib_type_t *attr_type,
                               size_t *size) {

    for (int i = 0; i < CETECH_ARRAY_LEN(_attrin_tbl); ++i) {
        if (strcmp(_attrin_tbl[i].name, name) != 0) {
            continue;
        }


        *attr_type = _attrin_tbl[i].attrib_type;
        *size = _attrin_tbl[i].size;
        return;
    }

    *attr_type = BGFX_ATTRIB_TYPE_COUNT;
    *size = 0;
}

void _parse_vertex_decl(bgfx_vertex_decl_t *decl,
                        uint32_t *vertex_size,
                        bgfx_attrib_t type,
                        yaml_node_t decl_node) {
    yaml_node_t type_n = yaml_get_node(decl_node, "type");
    yaml_node_t size_n = yaml_get_node(decl_node, "size");

    char type_str[64] = {0};
    yaml_as_string(type_n, type_str, CETECH_ARRAY_LEN(type_str));

    bgfx_attrib_type_t attrib_type;
    size_t v_size;

    _type_to_attr_type(type_str, &attrib_type, &v_size);

    *vertex_size += yaml_as_int(size_n) * v_size;

    bgfx_vertex_decl_add(decl, type, (uint8_t) yaml_as_int(size_n), attrib_type,
                         0, 0);
}


static void _parese_types(bgfx_vertex_decl_t *decl,
                          yaml_node_t types,
                          uint32_t *vertex_size) {

    for (int i = 0; i < CETECH_ARRAY_LEN(_chanel_types); ++i) {
        YAML_NODE_SCOPE(node, types, _chanel_types[i].name,
                        if (yaml_is_valid(node))
                            _parse_vertex_decl(decl, vertex_size,
                                               _chanel_types[i].attrib, node););

    }
}

void _write_chanel(yaml_node_t node,
                   yaml_node_t types,
                   size_t i,
                   const char *name,
                   yaml_node_t chanels_n,
                   struct compile_output *output) {
    char tmp_buff[64] = {0};
    bgfx_attrib_type_t attrib_type;
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

    uint64_t name = hash_api_v0.id64_from_str(name_str);

    array::push_back(output->geom_name, name);
    array::push_back<uint64_t>(output->geom_node, 0);
    array::push_back(output->ib_offset, array::size(output->ib));
    array::push_back(output->vb_offset, array::size(output->vb));

    // DECL
    bgfx_vertex_decl_t vertex_decl;
    bgfx_vertex_decl_begin(&vertex_decl, BGFX_RENDERER_TYPE_NOOP);

    yaml_node_t types = yaml_get_node(value, "types");
    uint32_t vertex_size = 0;
    _parese_types(&vertex_decl, types, &vertex_size);

    bgfx_vertex_decl_end(&vertex_decl);
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
                                _write_chanel(node, types, i, name, chanels_n,
                                              output););
        }

        array::push_back(output->ib, (uint32_t)i);
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
    uint64_t node_name = hash_api_v0.id64_from_str(buffer);

    yaml_node_t local_pose = yaml_get_node(value, "local");
    mat44f_t pose = yaml_as_mat44f_t(local_pose);

    uint32_t idx = (uint32_t) array::size(output->output->node_name);

    array::push_back(output->output->node_name, node_name);
    array::push_back(output->output->node_parent, output->parent_idx);
    array::push_back(output->output->node_pose, pose);

    yaml_node_t geometries_n = yaml_get_node(value, "geometries");
    if (yaml_is_valid(geometries_n)) {
        const size_t name_count = yaml_node_size(geometries_n);
        for (int i = 0; i < name_count; ++i) {
            yaml_node_t name_node = yaml_get_seq_node(geometries_n, i);
            yaml_as_string(name_node, buffer, CETECH_ARRAY_LEN(buffer));
            yaml_node_free(name_node);

            uint64_t geom_name = hash_api_v0.id64_from_str(buffer);
            for (int j = 0; j < array::size(output->output->geom_name); ++j) {
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
    uint64_t name = hash_api_v0.id64_from_str(root->mName.data);

    uint32_t idx = array::size(output->node_name);

    array::push_back(output->node_name, name);
    array::push_back(output->node_parent, parent);
    array::push_back(output->node_pose, *((mat44f_t *) &root->mTransformation));

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
                    struct compilator_api *capi) {
    yaml_node_t import_n = yaml_get_node(root, "import");
    yaml_node_t input_n = yaml_get_node(import_n, "input");

    yaml_node_t postprocess_n = yaml_get_node(import_n, "postprocess");

    char input_str[64] = {0};
    yaml_as_string(input_n, input_str, CETECH_ARRAY_LEN(input_str));
    capi->add_dependency(filename, input_str);

    char input_path[128] = {0};
    const char *source_dir = resource_api_v0.compiler_get_source_dir();
    path_v0.join(input_path, CETECH_ARRAY_LEN(input_path), source_dir,
                      input_str);

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
            snprintf(tmp_buffer, CETECH_ARRAY_LEN(tmp_buffer), "geom_%d", i);
        } else {
            memcpy(tmp_buffer, mesh->mName.data, mesh->mName.length);
        }

        uint64_t name_id = hash_api_v0.id64_from_str(tmp_buffer);
        for (int k = 0; k < array::size(output->geom_name); ++k) {
            if (name_id == output->geom_name[k]) {
                snprintf(tmp_buffer2, CETECH_ARRAY_LEN(tmp_buffer2), "%s%d",
                         tmp_buffer, ++unique);
                snprintf(tmp_buffer, CETECH_ARRAY_LEN(tmp_buffer), "%s",
                         tmp_buffer2);
                break;
            }
        }

        array::push_back(output->geom_name, hash_api_v0.id64_from_str(tmp_buffer));
        array::push_back<uint64_t >(output->geom_node, 0);
        array::push_back(output->ib_offset, array::size(output->ib));
        array::push_back(output->vb_offset, array::size(output->vb));
        array::push_back(output->ib_size, mesh->mNumFaces * 3);

        bgfx_vertex_decl_t vertex_decl;
        bgfx_vertex_decl_begin(&vertex_decl, BGFX_RENDERER_TYPE_NOOP);

        uint32_t v_size = 0;
        if (mesh->mVertices != NULL) {
            bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_POSITION, 3,
                                 BGFX_ATTRIB_TYPE_FLOAT, 0, 0);
            v_size += 3 * sizeof(float);
        }

        if (mesh->mNormals != NULL) {
            bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_NORMAL, 3,
                                 BGFX_ATTRIB_TYPE_FLOAT, 0, 0);
            v_size += 3 * sizeof(float);
        }

        if (mesh->mTextureCoords[0] != NULL) {
            bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_TEXCOORD0, 2,
                                 BGFX_ATTRIB_TYPE_FLOAT, 0, 0);
            v_size += 2 * sizeof(float);
        }
        bgfx_vertex_decl_end(&vertex_decl);

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
                array::push(output->vb, (uint8_t *) &mesh->mTextureCoords[0][j],
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

int _scene_resource_compiler(const char *filename,
                             struct vio *source_vio,
                             struct vio *build_vio,
                             struct compilator_api *compilator_api) {

    char *source_data =
    CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                    vio_api_v0.size(source_vio) + 1);
    memset(source_data, 0, vio_api_v0.size(source_vio) + 1);
    vio_api_v0.read(source_vio, source_data, sizeof(char),
                    vio_api_v0.size(source_vio));

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
        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), source_data);
        return 0;
    }

    struct scene_blob res = {
            .geom_count = (uint32_t) array::size(output->geom_name),
            .node_count = (uint32_t) array::size(output->node_name),
            .ib_len = (uint32_t) array::size(output->ib),
            .vb_len = (uint32_t) array::size(output->vb),
    };

    vio_api_v0.write(build_vio, &res, sizeof(res), 1);
    vio_api_v0.write(build_vio, array::begin(output->geom_name), sizeof(uint64_t),
                     array::size(output->geom_name));
    vio_api_v0.write(build_vio, array::begin(output->ib_offset), sizeof(uint32_t),
                     array::size(output->ib_offset));
    vio_api_v0.write(build_vio, array::begin(output->vb_offset), sizeof(uint32_t),
                     array::size(output->vb_offset));
    vio_api_v0.write(build_vio, array::begin(output->vb_decl),
                     sizeof(bgfx_vertex_decl_t),
                     array::size(output->vb_decl));
    vio_api_v0.write(build_vio, array::begin(output->ib_size), sizeof(uint32_t),
                     array::size(output->ib_size));
    vio_api_v0.write(build_vio, array::begin(output->vb_size), sizeof(uint32_t),
                     array::size(output->vb_size));
    vio_api_v0.write(build_vio, array::begin(output->ib), sizeof(uint32_t),
                     array::size(output->ib));
    vio_api_v0.write(build_vio, array::begin(output->vb), sizeof(uint8_t),
                     array::size(output->vb));
    vio_api_v0.write(build_vio, array::begin(output->node_name), sizeof(uint64_t),
                     array::size(output->node_name));
    vio_api_v0.write(build_vio, array::begin(output->node_parent), sizeof(uint32_t),
                     array::size(output->node_parent));
    vio_api_v0.write(build_vio, array::begin(output->node_pose), sizeof(mat44f_t),
                     array::size(output->node_pose));
    vio_api_v0.write(build_vio, array::begin(output->geom_node), sizeof(uint64_t),
                     array::size(output->geom_name));

    _destroy_compile_output(output);
    CETECH_DEALLOCATE(memory_api_v0.main_allocator(), source_data);
    return 1;
}

#endif //CETECH_SCENE_COMPILER_H
