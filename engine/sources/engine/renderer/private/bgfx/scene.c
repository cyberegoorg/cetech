//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <celib/string/stringid.h>
#include <engine/world/scenegraph.h>
#include "celib/filesystem/path.h"
#include "celib/filesystem/vio.h"
#include <engine/memory/memsys.h>

#include "engine/resource/resource.h"


//==============================================================================
// Structs
//==============================================================================

ARRAY_PROTOTYPE(bgfx_texture_handle_t)

ARRAY_PROTOTYPE(bgfx_vertex_buffer_handle_t);
ARRAY_PROTOTYPE(bgfx_index_buffer_handle_t);
ARRAY_PROTOTYPE(cel_mat44f_t);

MAP_PROTOTYPE(bgfx_texture_handle_t)

typedef struct scene_blob {
    u32 geom_count;
    u32 node_count;
    u32 ib_len;
    u32 vb_len;
    // stringid64_t       geom_name[geom_count]
    // u32                ib_offset[geom_count]
    // u32                vb_offset[geom_count]
    // bgfx_vertex_decl_t vb_decl[geom_count]
    // u32                ib_size [geom_count]
    // u32                vb_size [geom_count]
    // u32                ib[ib_len]
    // u8                 vb[vb_len]
    // stringid64_t       node_name[node_count]
    // u32                node_parent[node_count]
    // cel_mat44f_t           node_pose[node_count]
    // stringid64_t       geom_node[geom_count]
} scene_blob_t;

#define scene_blob_geom_name(r)   ((stringid64_t*)       ((r) + 1))
#define scene_blob_ib_offset(r)   ((u32*)                ((scene_blob_geom_name(r))   + (r)->geom_count))
#define scene_blob_vb_offset(r)   ((u32*)                ((scene_blob_ib_offset(r))   + (r)->geom_count))
#define scene_blob_vb_decl(r)     ((bgfx_vertex_decl_t*) ((scene_blob_vb_offset(r))   + (r)->geom_count))
#define scene_blob_ib_size(r)     ((u32*)                ((scene_blob_vb_decl(r))     + (r)->geom_count))
#define scene_blob_vb_size(r)     ((u32*)                ((scene_blob_ib_size(r))     + (r)->geom_count))
#define scene_blob_ib(r)          ((u32*)                ((scene_blob_vb_size(r))     + (r)->geom_count))
#define scene_blob_vb(r)          ((u8*)                 ((scene_blob_ib(r))          + (r)->ib_len))
#define scene_blob_node_name(r)   ((stringid64_t*)       ((scene_blob_vb(r))          + (r)->vb_len))
#define scene_blob_node_parent(r) ((u32*)                ((scene_blob_node_name(r))   + (r)->node_count))
#define scene_blob_node_pose(r)   ((cel_mat44f_t*)           ((scene_blob_node_parent(r)) + (r)->node_count))
#define scene_blob_geom_node(r)   ((stringid64_t*)       ((scene_blob_node_pose(r))   + (r)->node_count))

struct scene_instance {
    MAP_T(u8) geom_map;
    ARRAY_T(u32) size;
    ARRAY_T(bgfx_vertex_buffer_handle_t) vb;
    ARRAY_T(bgfx_index_buffer_handle_t) ib;
};

void _init_scene_instance(struct scene_instance *instance) {
    MAP_INIT(u8, &instance->geom_map, memsys_main_allocator());
    ARRAY_INIT(u32, &instance->size, memsys_main_allocator());
    ARRAY_INIT(bgfx_vertex_buffer_handle_t, &instance->vb, memsys_main_allocator());
    ARRAY_INIT(bgfx_index_buffer_handle_t, &instance->ib, memsys_main_allocator());
}

void _destroy_scene_instance(struct scene_instance *instance) {
    MAP_DESTROY(u8, &instance->geom_map);
    ARRAY_DESTROY(u32, &instance->size);
    ARRAY_DESTROY(bgfx_vertex_buffer_handle_t, &instance->vb);
    ARRAY_DESTROY(bgfx_index_buffer_handle_t, &instance->ib);
}


ARRAY_PROTOTYPE_N(struct scene_instance, scene_instance);
MAP_PROTOTYPE_N(struct scene_instance, scene_instance);

//==============================================================================
// GLobals
//==============================================================================

#define _G SceneResourceGlobals
static struct G {
    stringid64_t type;
    MAP_T(scene_instance) scene_instance;
} _G = {0};

struct scene_instance *_get_scene_instance(stringid64_t scene) {
    return MAP_GET_PTR(scene_instance, &_G.scene_instance, scene.id);
}

//==============================================================================
// Compiler private
//==============================================================================

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
    struct cel_allocator *a = memsys_main_allocator();
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
    struct cel_allocator *a = memsys_main_allocator();

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

    bgfx_vertex_decl_add(decl, type, (uint8_t) yaml_as_int(size_n), attrib_type, 0, 0);
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
                        if (yaml_is_valid(node)) _parse_vertex_decl(decl, vertex_size, _chanel_types[i].attrib, node););

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
                            if (yaml_is_valid(node))_write_chanel(node, types, i, name, chanels_n, output););
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
                if (geom_name.id != ARRAY_AT(&output->output->geom_name, j).id) {
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
    ARRAY_PUSH_BACK(cel_mat44f_t, &output->node_pose, *((cel_mat44f_t *) &root->mTransformation));

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
    const char *source_dir = resource_compiler_get_source_dir();
    cel_path_join(input_path, CEL_ARRAY_LEN(input_path), source_dir, input_str);

    u32 postprocess_flag = aiProcessPreset_TargetRealtime_MaxQuality;

    if (yaml_is_valid(postprocess_n)) {
        YAML_NODE_SCOPE(node, postprocess_n, "flip_uvs",
                        if (yaml_is_valid(node) && yaml_as_bool(node)) postprocess_flag |= aiProcess_FlipUVs;);
    }

    const struct aiScene *scene = aiImportFile(input_path, postprocess_flag | aiProcess_MakeLeftHanded);

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
                snprintf(tmp_buffer2, CEL_ARRAY_LEN(tmp_buffer2), "%s%d", tmp_buffer, ++unique);
                snprintf(tmp_buffer, CEL_ARRAY_LEN(tmp_buffer), "%s", tmp_buffer2);
                break;
            }
        }

        ARRAY_PUSH_BACK(stringid64_t, &output->geom_name, stringid64_from_string(tmp_buffer));
        ARRAY_PUSH_BACK(stringid64_t, &output->geom_node, (stringid64_t) {.id = 0});
        ARRAY_PUSH_BACK(u32, &output->ib_offset, ARRAY_SIZE(&output->ib));
        ARRAY_PUSH_BACK(u32, &output->vb_offset, ARRAY_SIZE(&output->vb));
        ARRAY_PUSH_BACK(u32, &output->ib_size, mesh->mNumFaces * 3);

        bgfx_vertex_decl_t vertex_decl;
        bgfx_vertex_decl_begin(&vertex_decl, BGFX_RENDERER_TYPE_NOOP);

        u32 v_size = 0;
        if (mesh->mVertices != NULL) {
            bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_POSITION, 3, BGFX_ATTRIB_TYPE_FLOAT, 0, 0);
            v_size += 3 * sizeof(f32);
        }

        if (mesh->mNormals != NULL) {
            bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_NORMAL, 3, BGFX_ATTRIB_TYPE_FLOAT, 0, 0);
            v_size += 3 * sizeof(f32);
        }

        if (mesh->mTextureCoords[0] != NULL) {
            bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_TEXCOORD0, 2, BGFX_ATTRIB_TYPE_FLOAT, 0, 0);
            v_size += 2 * sizeof(f32);
        }
        bgfx_vertex_decl_end(&vertex_decl);
        ARRAY_PUSH_BACK(bgfx_vertex_decl_t, &output->vb_decl, vertex_decl);
        ARRAY_PUSH_BACK(u32, &output->vb_size, v_size * mesh->mNumVertices);

        for (int j = 0; j < mesh->mNumVertices; ++j) {
            if (mesh->mVertices != NULL) {
                ARRAY_PUSH(u8, &output->vb, (u8 *) &mesh->mVertices[j], sizeof(f32) * 3);
            }

            if (mesh->mNormals != NULL) {
                ARRAY_PUSH(u8, &output->vb, (u8 *) &mesh->mNormals[j], sizeof(f32) * 3);
            }

            if (mesh->mTextureCoords[0] != NULL) {
                ARRAY_PUSH(u8, &output->vb, (u8 *) &mesh->mTextureCoords[0][j], sizeof(f32) * 2);
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

    char *source_data = CEL_ALLOCATE(memsys_main_allocator(), char, cel_vio_size(source_vio) + 1);
    memory_set(source_data, 0, cel_vio_size(source_vio) + 1);
    cel_vio_read(source_vio, source_data, sizeof(char), cel_vio_size(source_vio));

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
        CEL_DEALLOCATE(memsys_main_allocator(), source_data);
        return 0;
    }

    struct scene_blob res = {
            .geom_count = (u32) ARRAY_SIZE(&output->geom_name),
            .node_count = (u32) ARRAY_SIZE(&output->node_name),
            .ib_len = (u32) ARRAY_SIZE(&output->ib),
            .vb_len = (u32) ARRAY_SIZE(&output->vb),
    };

    cel_vio_write(build_vio, &res, sizeof(res), 1);
    cel_vio_write(build_vio, output->geom_name.data, sizeof(stringid64_t), ARRAY_SIZE(&output->geom_name));
    cel_vio_write(build_vio, output->ib_offset.data, sizeof(u32), ARRAY_SIZE(&output->ib_offset));
    cel_vio_write(build_vio, output->vb_offset.data, sizeof(u32), ARRAY_SIZE(&output->vb_offset));
    cel_vio_write(build_vio, output->vb_decl.data, sizeof(bgfx_vertex_decl_t), ARRAY_SIZE(&output->vb_decl));
    cel_vio_write(build_vio, output->ib_size.data, sizeof(u32), ARRAY_SIZE(&output->ib_size));
    cel_vio_write(build_vio, output->vb_size.data, sizeof(u32), ARRAY_SIZE(&output->vb_size));
    cel_vio_write(build_vio, output->ib.data, sizeof(u32), ARRAY_SIZE(&output->ib));
    cel_vio_write(build_vio, output->vb.data, sizeof(u8), ARRAY_SIZE(&output->vb));
    cel_vio_write(build_vio, output->node_name.data, sizeof(stringid64_t), ARRAY_SIZE(&output->node_name));
    cel_vio_write(build_vio, output->node_parent.data, sizeof(u32), ARRAY_SIZE(&output->node_parent));
    cel_vio_write(build_vio, output->node_pose.data, sizeof(cel_mat44f_t), ARRAY_SIZE(&output->node_pose));
    cel_vio_write(build_vio, output->geom_node.data, sizeof(stringid64_t), ARRAY_SIZE(&output->geom_name));

    _destroy_compile_output(output);
    CEL_DEALLOCATE(memsys_main_allocator(), source_data);
    return 1;
}

//==============================================================================
// Resource
//==============================================================================

void *scene_resource_loader(struct vio *input,
                            struct cel_allocator *allocator) {
    const i64 size = cel_vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    cel_vio_read(input, data, 1, size);

    return data;
}

void scene_resource_unloader(void *new_data,
                             struct cel_allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
}

void scene_resource_online(stringid64_t name,
                           void *data) {
    struct scene_blob *resource = data;

    stringid64_t *geom_name = scene_blob_geom_name(resource);
    u32 *ib_offset = scene_blob_ib_offset(resource);
    u32 *vb_offset = scene_blob_vb_offset(resource);
    bgfx_vertex_decl_t *vb_decl = scene_blob_vb_decl(resource);
    u32 *ib_size = scene_blob_ib_size(resource);
    u32 *vb_size = scene_blob_vb_size(resource);
    u32 *ib = scene_blob_ib(resource);
    u8 *vb = scene_blob_vb(resource);

    struct scene_instance instance = {0};
    _init_scene_instance(&instance);

    for (int i = 0; i < resource->geom_count; ++i) {
        bgfx_vertex_buffer_handle_t bvb = bgfx_create_vertex_buffer(
                bgfx_make_ref((const void *) &vb[vb_offset[i]], vb_size[i]), &vb_decl[i], BGFX_BUFFER_NONE);

        bgfx_index_buffer_handle_t bib = bgfx_create_index_buffer(
                bgfx_make_ref((const void *) &ib[ib_offset[i]], sizeof(u32) * ib_size[i]), BGFX_BUFFER_INDEX32);

        u32 idx = ARRAY_SIZE(&instance.vb);
        MAP_SET(u8, &instance.geom_map, geom_name[i].id, idx);

        ARRAY_PUSH_BACK(u32, &instance.size, ib_size[i]);
        ARRAY_PUSH_BACK(bgfx_vertex_buffer_handle_t, &instance.vb, bvb);
        ARRAY_PUSH_BACK(bgfx_index_buffer_handle_t, &instance.ib, bib);
    }

    MAP_SET(scene_instance, &_G.scene_instance, name.id, instance);
}

static const bgfx_texture_handle_t null_texture = {0};

void scene_resource_offline(stringid64_t name,
                            void *data) {
//    bgfx_texture_handle_t texture = MAP_GET(bgfx_texture_handle_t, &_G.handler_map, name.id, null_texture);
//
//    if (texture.idx == null_texture.idx) {
//        return;
//    }
//
//    bgfx_destroy_texture(texture);

}

void *scene_resource_reloader(stringid64_t name,
                              void *old_data,
                              void *new_data,
                              struct cel_allocator *allocator) {
    scene_resource_offline(name, old_data);
    scene_resource_online(name, new_data);

    CEL_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t scene_resource_callback = {
        .loader = scene_resource_loader,
        .unloader =scene_resource_unloader,
        .online =scene_resource_online,
        .offline =scene_resource_offline,
        .reloader = scene_resource_reloader
};


//==============================================================================
// Interface
//==============================================================================

int scene_resource_init() {
    _G = (struct G) {0};

    _G.type = stringid64_from_string("scene");

    MAP_INIT(scene_instance, &_G.scene_instance, memsys_main_allocator());

    resource_compiler_register(_G.type, _scene_resource_compiler);
    resource_register_type(_G.type, scene_resource_callback);

    return 1;
}

void scene_resource_shutdown() {
    MAP_DESTROY(scene_instance, &_G.scene_instance);
    _G = (struct G) {0};
}

void scene_resource_submit(stringid64_t scene,
                           stringid64_t geom_name) {
    resource_get(_G.type, scene);
    struct scene_instance *instance = _get_scene_instance(scene);

    if (instance == NULL) {
        return;
    }

    u8 idx = MAP_GET(u8, &instance->geom_map, geom_name.id, UINT8_MAX);

    if (idx == UINT8_MAX) {
        return;
    }

    bgfx_set_vertex_buffer(instance->vb.data[idx], 0, instance->size.data[idx]);
    bgfx_set_index_buffer(instance->ib.data[idx], 0, instance->size.data[idx]);
}

void scene_create_graph(world_t world,
                        entity_t entity,
                        stringid64_t scene) {
    struct scene_blob *res = resource_get(_G.type, scene);

    stringid64_t *node_name = scene_blob_node_name(res);
    u32 *node_parent = scene_blob_node_parent(res);
    cel_mat44f_t *node_pose = scene_blob_node_pose(res);

    scenegraph_create(world, entity, node_name, node_parent, node_pose, res->node_count);
}

stringid64_t scene_get_mesh_node(stringid64_t scene,
                                 stringid64_t mesh) {
    struct scene_blob *res = resource_get(_G.type, scene);

    stringid64_t *geom_node = scene_blob_geom_node(res);
    stringid64_t *geom_name = scene_blob_geom_name(res);

    for (int i = 0; i < res->geom_count; ++i) {
        if (geom_name[i].id != mesh.id) {
            continue;
        }

        return geom_node[i];
    }

    return (stringid64_t) {0};
}