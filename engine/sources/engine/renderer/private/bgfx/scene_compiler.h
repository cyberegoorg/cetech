#ifndef CETECH_SCENE_COMPILER_H
#define CETECH_SCENE_COMPILER_H

ARRAY_PROTOTYPE(bgfx_vertex_decl_t);
ARRAY_PROTOTYPE(stringid64_t);

struct compile_output {
    ARRAY_T(stringid64_t) geom_name;
    ARRAY_T(u32) ib_offset;
    ARRAY_T(u32) vb_offset;
    ARRAY_T(bgfx_vertex_decl_t) vb_decl;
    ARRAY_T(u32) ib_size;
    ARRAY_T(u32) vb_size;
    ARRAY_T(u32) ib;
    ARRAY_T(u8) vb;

    ARRAY_T(stringid64_t) node_name;
    ARRAY_T(u32) node_parent;
    ARRAY_T(cel_mat44f_t) node_pose;
    ARRAY_T(stringid64_t) geom_node;
};

struct compile_output *_crete_compile_output() {
    struct cel_allocator *a = MemSysApiV0.main_allocator();
    struct compile_output *output = CEL_ALLOCATE(a, struct compile_output, 1);

    ARRAY_INIT(stringid64_t, &output->geom_name, a);
    ARRAY_INIT(u32, &output->ib_offset, a);
    ARRAY_INIT(u32, &output->vb_offset, a);
    ARRAY_INIT(bgfx_vertex_decl_t, &output->vb_decl, a);
    ARRAY_INIT(u32, &output->ib_size, a);
    ARRAY_INIT(u32, &output->vb_size, a);
    ARRAY_INIT(u32, &output->ib, a);
    ARRAY_INIT(u8, &output->vb, a);

    ARRAY_INIT(stringid64_t, &output->node_name, a);
    ARRAY_INIT(stringid64_t, &output->geom_node, a);
    ARRAY_INIT(u32, &output->node_parent, a);
    ARRAY_INIT(cel_mat44f_t, &output->node_pose, a);


    return output;
}

void _destroy_compile_output(struct compile_output *output) {
    struct cel_allocator *a = MemSysApiV0.main_allocator();

    ARRAY_DESTROY(stringid64_t, &output->geom_name);
    ARRAY_DESTROY(u32, &output->ib_offset);
    ARRAY_DESTROY(u32, &output->vb_offset);
    ARRAY_DESTROY(bgfx_vertex_decl_t, &output->vb_decl);
    ARRAY_DESTROY(u32, &output->ib_size);
    ARRAY_DESTROY(u32, &output->vb_size);
    ARRAY_DESTROY(u32, &output->ib);
    ARRAY_DESTROY(u8, &output->vb);

    ARRAY_DESTROY(stringid64_t, &output->node_name);
    ARRAY_DESTROY(stringid64_t, &output->geom_node);
    ARRAY_DESTROY(u32, &output->node_parent);
    ARRAY_DESTROY(cel_mat44f_t, &output->node_pose);

    CEL_DEALLOCATE(a, output);
}

static void _type_to_attr_type(const char *name,
                               bgfx_attrib_type_t *attr_type,
                               size_t *size) {
    static const struct {
        const char *name;
        bgfx_attrib_type_t attrib_type;
        size_t size;
    } _tbl[] = {
            {.name="f32", .size=sizeof(f32), .attrib_type=BGFX_ATTRIB_TYPE_FLOAT},
            {.name="i16", .size=sizeof(i16), .attrib_type=BGFX_ATTRIB_TYPE_INT16},
            {.name="u8", .size=sizeof(u8), .attrib_type=BGFX_ATTRIB_TYPE_UINT8},
            // TODO: {.name="f16", .size=sizeof(f16), .attrib_type=BGFX_ATTRIB_TYPE_HALF},
            // TODO: {.name="u10", .size=sizeof(u10), .attrib_type=BGFX_ATTRIB_TYPE_UINT10},
    };

    for (int i = 0; i < CEL_ARRAY_LEN(_tbl); ++i) {
        if (cel_strcmp(_tbl[i].name, name) != 0) {
            continue;
        }


        *attr_type = _tbl[i].attrib_type;
        *size = _tbl[i].size;
        return;
    }

    *attr_type = BGFX_ATTRIB_TYPE_COUNT;
    *size = 0;
}

void _parse_vertex_decl(bgfx_vertex_decl_t *decl,
                        u32 *vertex_size,
                        bgfx_attrib_t type,
                        yaml_node_t decl_node) {
    yaml_node_t type_n = yaml_get_node(decl_node, "type");
    yaml_node_t size_n = yaml_get_node(decl_node, "size");

    char type_str[64] = {0};
    yaml_as_string(type_n, type_str, CEL_ARRAY_LEN(type_str));

    bgfx_attrib_type_t attrib_type;
    size_t v_size;

    _type_to_attr_type(type_str, &attrib_type, &v_size);

    *vertex_size += yaml_as_int(size_n) * v_size;

    bgfx_vertex_decl_add(decl, type, (uint8_t) yaml_as_int(size_n), attrib_type,
                         0, 0);
}

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

static void _parese_types(bgfx_vertex_decl_t *decl,
                          yaml_node_t types,
                          u32 *vertex_size) {

    for (int i = 0; i < CEL_ARRAY_LEN(_chanel_types); ++i) {
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
    u32 idx = yaml_as_int(idx_n);
    yaml_node_free(idx_n);

    u32 size = 0;

    {
        yaml_node_t chan_def_n = yaml_get_node(types, name);
        yaml_node_t type_n = yaml_get_node(chan_def_n, "type");
        yaml_node_t size_n = yaml_get_node(chan_def_n, "size");

        yaml_as_string(type_n, tmp_buff, CEL_ARRAY_LEN(tmp_buff));
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
        f32 v = yaml_as_float(n);
        yaml_node_free(n);

        //log_debug("casdsadsa", "%s:%d -  %f", name, k, v);

        ARRAY_PUSH(u8, &output->vb, (u8 *) &v, sizeof(v));
    }
    yaml_node_free(chanel_data_n);
}

void foreach_geometries_clb(yaml_node_t key,
                            yaml_node_t value,
                            void *_data) {
    struct compile_output *output = _data;

    char name_str[64] = {0};
    yaml_as_string(key, name_str, CEL_ARRAY_LEN(name_str));

    stringid64_t name = stringid64_from_string(name_str);

    ARRAY_PUSH_BACK(stringid64_t, &output->geom_name, name);
    ARRAY_PUSH_BACK(stringid64_t, &output->geom_node, (stringid64_t) {.id = 0});
    ARRAY_PUSH_BACK(u32, &output->ib_offset, ARRAY_SIZE(&output->ib));
    ARRAY_PUSH_BACK(u32, &output->vb_offset, ARRAY_SIZE(&output->vb));

    // DECL
    bgfx_vertex_decl_t vertex_decl;
    bgfx_vertex_decl_begin(&vertex_decl, BGFX_RENDERER_TYPE_NOOP);

    yaml_node_t types = yaml_get_node(value, "types");
    u32 vertex_size = 0;
    _parese_types(&vertex_decl, types, &vertex_size);

    bgfx_vertex_decl_end(&vertex_decl);
    ARRAY_PUSH_BACK(bgfx_vertex_decl_t, &output->vb_decl, vertex_decl);


    // IB, VB
    yaml_node_t chanels_n = yaml_get_node(value, "chanels");
    yaml_node_t indices_n = yaml_get_node(value, "indices");
    yaml_node_t i_size = yaml_get_node(indices_n, "size");

    u32 vertex_count = yaml_as_int(i_size);

    ARRAY_PUSH_BACK(u32, &output->ib_size, vertex_count);
    ARRAY_PUSH_BACK(u32, &output->vb_size, vertex_size * vertex_count);

    for (int i = 0; i < vertex_count; ++i) {
        for (int j = 0; j < CEL_ARRAY_LEN(_chanel_types); ++j) {
            const char *name = _chanel_types[j].name;

            YAML_NODE_SCOPE(node, indices_n, name,
                            if (yaml_is_valid(node))
                                _write_chanel(node, types, i, name, chanels_n,
                                              output););
        }

        ARRAY_PUSH_BACK(u32, &output->ib, i);
    }
}


struct foreach_graph_data {
    struct compile_output *output;
    u32 parent_idx;
};


void foreach_graph_clb(yaml_node_t key,
                       yaml_node_t value,
                       void *_data) {
    char buffer[128] = {0};
    struct foreach_graph_data *output = _data;

    yaml_as_string(key, buffer, CEL_ARRAY_LEN(buffer));
    stringid64_t node_name = stringid64_from_string(buffer);

    yaml_node_t local_pose = yaml_get_node(value, "local");
    cel_mat44f_t pose = yaml_as_cel_mat44f_t(local_pose);

    u32 idx = (u32) ARRAY_SIZE(&output->output->node_name);

    ARRAY_PUSH_BACK(stringid64_t, &output->output->node_name, node_name);
    ARRAY_PUSH_BACK(u32, &output->output->node_parent, output->parent_idx);
    ARRAY_PUSH_BACK(cel_mat44f_t, &output->output->node_pose, pose);

    yaml_node_t geometries_n = yaml_get_node(value, "geometries");
    if (yaml_is_valid(geometries_n)) {
        const size_t name_count = yaml_node_size(geometries_n);
        for (int i = 0; i < name_count; ++i) {
            yaml_node_t name_node = yaml_get_seq_node(geometries_n, i);
            yaml_as_string(name_node, buffer, CEL_ARRAY_LEN(buffer));
            yaml_node_free(name_node);

            stringid64_t geom_name = stringid64_from_string(buffer);
            for (int j = 0; j < ARRAY_SIZE(&output->output->geom_name); ++j) {
                if (geom_name.id !=
                    ARRAY_AT(&output->output->geom_name, j).id) {
                    continue;
                }

                ARRAY_AT(&output->output->geom_node, j) = node_name;
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
                          u32 parent,
                          struct compile_output *output) {
    stringid64_t name = stringid64_from_string(root->mName.data);

    u32 idx = ARRAY_SIZE(&output->node_name);

    ARRAY_PUSH_BACK(stringid64_t, &output->node_name, name);
    ARRAY_PUSH_BACK(u32, &output->node_parent, parent);
    ARRAY_PUSH_BACK(cel_mat44f_t, &output->node_pose,
                    *((cel_mat44f_t *) &root->mTransformation));

    for (int i = 0; i < root->mNumChildren; ++i) {
        _compile_assimp_node(root->mChildren[i], idx, output);
    }

    for (int i = 0; i < root->mNumMeshes; ++i) {
        ARRAY_PUSH_BACK(stringid64_t, &output->geom_node, name);
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
    yaml_as_string(input_n, input_str, CEL_ARRAY_LEN(input_str));
    capi->add_dependency(filename, input_str);

    char input_path[128] = {0};
    const char *source_dir = ResourceApiV0.compiler_get_source_dir();
    cel_path_join(input_path, CEL_ARRAY_LEN(input_path), source_dir, input_str);

    u32 postprocess_flag = aiProcessPreset_TargetRealtime_MaxQuality;

    if (yaml_is_valid(postprocess_n)) {
        YAML_NODE_SCOPE(node, postprocess_n, "flip_uvs",
                        if (yaml_is_valid(node) &&
                            yaml_as_bool(node))
                            postprocess_flag |= aiProcess_FlipUVs;);
    }

    const struct aiScene *scene = aiImportFile(input_path, postprocess_flag |
                                                           aiProcess_MakeLeftHanded);

    char tmp_buffer[1024] = {0};
    char tmp_buffer2[1024] = {0};
    u32 unique = 0;
    for (int i = 0; i < scene->mNumMeshes; ++i) {
        struct aiMesh *mesh = scene->mMeshes[i];

        if (mesh->mName.length == 0) {
            snprintf(tmp_buffer, CEL_ARRAY_LEN(tmp_buffer), "geom_%d", i);
        } else {
            memory_copy(tmp_buffer, mesh->mName.data, mesh->mName.length);
        }

        stringid64_t name_id = stringid64_from_string(tmp_buffer);
        for (int k = 0; k < ARRAY_SIZE(&output->geom_name); ++k) {
            if (name_id.id == ARRAY_AT(&output->geom_name, k).id) {
                snprintf(tmp_buffer2, CEL_ARRAY_LEN(tmp_buffer2), "%s%d",
                         tmp_buffer, ++unique);
                snprintf(tmp_buffer, CEL_ARRAY_LEN(tmp_buffer), "%s",
                         tmp_buffer2);
                break;
            }
        }

        ARRAY_PUSH_BACK(stringid64_t, &output->geom_name,
                        stringid64_from_string(tmp_buffer));
        ARRAY_PUSH_BACK(stringid64_t, &output->geom_node,
                        (stringid64_t) {.id = 0});
        ARRAY_PUSH_BACK(u32, &output->ib_offset, ARRAY_SIZE(&output->ib));
        ARRAY_PUSH_BACK(u32, &output->vb_offset, ARRAY_SIZE(&output->vb));
        ARRAY_PUSH_BACK(u32, &output->ib_size, mesh->mNumFaces * 3);

        bgfx_vertex_decl_t vertex_decl;
        bgfx_vertex_decl_begin(&vertex_decl, BGFX_RENDERER_TYPE_NOOP);

        u32 v_size = 0;
        if (mesh->mVertices != NULL) {
            bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_POSITION, 3,
                                 BGFX_ATTRIB_TYPE_FLOAT, 0, 0);
            v_size += 3 * sizeof(f32);
        }

        if (mesh->mNormals != NULL) {
            bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_NORMAL, 3,
                                 BGFX_ATTRIB_TYPE_FLOAT, 0, 0);
            v_size += 3 * sizeof(f32);
        }

        if (mesh->mTextureCoords[0] != NULL) {
            bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_TEXCOORD0, 2,
                                 BGFX_ATTRIB_TYPE_FLOAT, 0, 0);
            v_size += 2 * sizeof(f32);
        }
        bgfx_vertex_decl_end(&vertex_decl);
        ARRAY_PUSH_BACK(bgfx_vertex_decl_t, &output->vb_decl, vertex_decl);
        ARRAY_PUSH_BACK(u32, &output->vb_size, v_size * mesh->mNumVertices);

        for (int j = 0; j < mesh->mNumVertices; ++j) {
            if (mesh->mVertices != NULL) {
                ARRAY_PUSH(u8, &output->vb, (u8 *) &mesh->mVertices[j],
                           sizeof(f32) * 3);
            }

            if (mesh->mNormals != NULL) {
                ARRAY_PUSH(u8, &output->vb, (u8 *) &mesh->mNormals[j],
                           sizeof(f32) * 3);
            }

            if (mesh->mTextureCoords[0] != NULL) {
                ARRAY_PUSH(u8, &output->vb, (u8 *) &mesh->mTextureCoords[0][j],
                           sizeof(f32) * 2);
            }
        }

        for (int j = 0; j < mesh->mNumFaces; ++j) {
            ARRAY_PUSH_BACK(u32, &output->ib, mesh->mFaces[j].mIndices[0]);
            ARRAY_PUSH_BACK(u32, &output->ib, mesh->mFaces[j].mIndices[1]);
            ARRAY_PUSH_BACK(u32, &output->ib, mesh->mFaces[j].mIndices[2]);
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

    char *source_data = CEL_ALLOCATE(MemSysApiV0.main_allocator(), char,
                                     cel_vio_size(source_vio) + 1);
    memory_set(source_data, 0, cel_vio_size(source_vio) + 1);
    cel_vio_read(source_vio, source_data, sizeof(char),
                 cel_vio_size(source_vio));

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
        CEL_DEALLOCATE(MemSysApiV0.main_allocator(), source_data);
        return 0;
    }

    struct scene_blob res = {
            .geom_count = (u32) ARRAY_SIZE(&output->geom_name),
            .node_count = (u32) ARRAY_SIZE(&output->node_name),
            .ib_len = (u32) ARRAY_SIZE(&output->ib),
            .vb_len = (u32) ARRAY_SIZE(&output->vb),
    };

    cel_vio_write(build_vio, &res, sizeof(res), 1);
    cel_vio_write(build_vio, output->geom_name.data, sizeof(stringid64_t),
                  ARRAY_SIZE(&output->geom_name));
    cel_vio_write(build_vio, output->ib_offset.data, sizeof(u32),
                  ARRAY_SIZE(&output->ib_offset));
    cel_vio_write(build_vio, output->vb_offset.data, sizeof(u32),
                  ARRAY_SIZE(&output->vb_offset));
    cel_vio_write(build_vio, output->vb_decl.data, sizeof(bgfx_vertex_decl_t),
                  ARRAY_SIZE(&output->vb_decl));
    cel_vio_write(build_vio, output->ib_size.data, sizeof(u32),
                  ARRAY_SIZE(&output->ib_size));
    cel_vio_write(build_vio, output->vb_size.data, sizeof(u32),
                  ARRAY_SIZE(&output->vb_size));
    cel_vio_write(build_vio, output->ib.data, sizeof(u32),
                  ARRAY_SIZE(&output->ib));
    cel_vio_write(build_vio, output->vb.data, sizeof(u8),
                  ARRAY_SIZE(&output->vb));
    cel_vio_write(build_vio, output->node_name.data, sizeof(stringid64_t),
                  ARRAY_SIZE(&output->node_name));
    cel_vio_write(build_vio, output->node_parent.data, sizeof(u32),
                  ARRAY_SIZE(&output->node_parent));
    cel_vio_write(build_vio, output->node_pose.data, sizeof(cel_mat44f_t),
                  ARRAY_SIZE(&output->node_pose));
    cel_vio_write(build_vio, output->geom_node.data, sizeof(stringid64_t),
                  ARRAY_SIZE(&output->geom_name));

    _destroy_compile_output(output);
    CEL_DEALLOCATE(MemSysApiV0.main_allocator(), source_data);
    return 1;
}

#endif //CETECH_SCENE_COMPILER_H
