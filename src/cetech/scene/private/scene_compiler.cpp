//==============================================================================
// Include
//==============================================================================
#include <time.h>

#include <celib/memory/allocator.h>
#include <celib/macros.h>
#include <celib/ydb.h>
#include <celib/containers/array.h>
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"

#include <celib/ydb.h>
#include <celib/cdb.h>
#include <celib/config.h>


#include "cetech/machine/machine.h"
#include "cetech/resource/resource.h"
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include "cetech/ecs/ecs.h"
#include <cetech/scene/scene.h>
#include <cetech/kernel/kernel.h>
#include <cetech/resource/resourcedb.h>

#include <include/assimp/scene.h>
#include <include/assimp/postprocess.h>
#include <include/assimp/cimport.h>
#include <celib/log.h>
#include <celib/containers/buffer.h>
#include <cetech/resource/resource_compiler.h>
#include <celib/os/path.h>


#define _G scene_compiler_globals

struct _G {
    ce_alloc_t0 *allocator;
} _G;

typedef char char_128[128];

struct compile_output {
    uint64_t *geom_name;
    uint32_t *ib_offset;
    uint32_t *vb_offset;
    bgfx_vertex_decl_t *vb_decl;
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
//    bgfx_attrib_t attrib;
//} _chanel_types[] = {
//        {.name="position", .attrib=BGFX_ATTRIB_POSITION},
//        {.name="normal", .attrib=BGFX_ATTRIB_NORMAL},
//        {.name="tangent", .attrib=BGFX_ATTRIB_TANGENT},
//        {.name="bitangent", .attrib=BGFX_ATTRIB_BITANGENT},
//        {.name="color0", .attrib=BGFX_ATTRIB_COLOR0},
//        {.name="color1", .attrib=BGFX_ATTRIB_COLOR1},
//        {.name="color2", .attrib=BGFX_ATTRIB_COLOR2},
//        {.name="color3", .attrib=BGFX_ATTRIB_COLOR3},
//        {.name="indices", .attrib=BGFX_ATTRIB_INDICES},
//        {.name="weight", .attrib=BGFX_ATTRIB_INDICES},
//        {.name="texcoord0", .attrib=BGFX_ATTRIB_TEXCOORD0},
//        {.name="texcoord1", .attrib=BGFX_ATTRIB_TEXCOORD1},
//        {.name="texcoord2", .attrib=BGFX_ATTRIB_TEXCOORD2},
//        {.name="texcoord3", .attrib=BGFX_ATTRIB_TEXCOORD3},
//        {.name="texcoord4", .attrib=BGFX_ATTRIB_TEXCOORD4},
//        {.name="texcoord5", .attrib=BGFX_ATTRIB_TEXCOORD5},
//        {.name="texcoord6", .attrib=BGFX_ATTRIB_TEXCOORD6},
//        {.name="texcoord7", .attrib=BGFX_ATTRIB_TEXCOORD7},
//};
//
//static const struct {
//    const char *name;
//    size_t size;
//    bgfx_attrib_type_t attrib_type;
//} _attrin_tbl[] = {
//        {.name="f32", .size=sizeof(float), .attrib_type=BGFX_ATTRIB_TYPE_FLOAT},
//        {.name="int16_t", .size=sizeof(int16_t), .attrib_type=BGFX_ATTRIB_TYPE_INT16},
//        {.name="uint8_t", .size=sizeof(uint8_t), .attrib_type=BGFX_ATTRIB_TYPE_UINT8},
////        {.name="float16_t", .size=sizeof(float16_t), .attrib_type=BGFX_ATTRIB_TYPE_HALF},
//        // TODO: {.name="u10", .size=sizeof(u10), .attrib_type=BGFX_ATTRIB_TYPE_UINT10},
//};
//

struct compile_output *_crete_compile_output() {
    compile_output *output =
            CE_ALLOC(_G.allocator, struct compile_output,
                     sizeof(compile_output));
    *output = {};

    return output;
}

static void _destroy_compile_output(compile_output *output) {
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

static void _compile_assimp_node(aiNode *root,
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

static int _compile_assimp(ce_cdb_t0 db,
                           uint64_t k,
                           struct compile_output *output) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, k);

    uint64_t import_obj = ce_cdb_a0->read_subobject(reader,
                                                    ce_id_a0->id64("import"),
                                                    0);

    const ce_cdb_obj_o0 *import_reader = ce_cdb_a0->read(db,
                                                        import_obj);

    const char *input_str = ce_cdb_a0->read_str(import_reader,
                                                ce_id_a0->id64("input"), "");

    const ce_cdb_obj_o0 *c_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                   ce_config_a0->obj());
    const char *source_dir = ce_cdb_a0->read_str(c_reader,
                                                 CONFIG_SRC, "");
    char *input_path = NULL;
    ce_os_path_a0->join(&input_path, _G.allocator, 2, source_dir,
                         input_str);

    uint32_t postprocess_flag = aiProcessPreset_TargetRealtime_MaxQuality
                                | aiProcess_ConvertToLeftHanded;

    uint64_t postprocess_obj = ce_cdb_a0->read_subobject(import_reader,
                                                         ce_id_a0->id64(
                                                                 "postprocess"),
                                                         0);

    const ce_cdb_obj_o0 *pp_reader = ce_cdb_a0->read(db,
                                                    postprocess_obj);

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

        bgfx_vertex_decl_t vertex_decl;
        ct_gfx_a0->bgfx_vertex_decl_begin(&vertex_decl,
                                          BGFX_RENDERER_TYPE_COUNT);

        uint32_t v_size = 0;
        if (mesh->mVertices != NULL) {
            ct_gfx_a0->bgfx_vertex_decl_add(&vertex_decl,
                                            BGFX_ATTRIB_POSITION, 3,
                                            BGFX_ATTRIB_TYPE_FLOAT, 0, 0);
            v_size += 3 * sizeof(float);
        }

        if (mesh->mNormals != NULL) {
            ct_gfx_a0->bgfx_vertex_decl_add(&vertex_decl,
                                            BGFX_ATTRIB_NORMAL, 3,
                                            BGFX_ATTRIB_TYPE_FLOAT, true,
                                            0);
            v_size += 3 * sizeof(float);
        }

        if (mesh->mTextureCoords[0] != NULL) {
            ct_gfx_a0->bgfx_vertex_decl_add(&vertex_decl,
                                            BGFX_ATTRIB_TEXCOORD0, 2,
                                            BGFX_ATTRIB_TYPE_FLOAT, 0, 0);

            v_size += 2 * sizeof(float);
        }
        ct_gfx_a0->bgfx_vertex_decl_end(&vertex_decl);

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

extern "C" bool scene_compiler(ce_cdb_t0 db,
                               uint64_t obj) {
    compile_output *output = _crete_compile_output();

    int ret = 1;

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, obj);

    if (ce_cdb_a0->prop_exist(reader, ce_id_a0->id64("import"))) {
        ret = _compile_assimp(db, obj, output);
    }

    if (!ret) {
        _destroy_compile_output(output);
        return false;
    }

    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, obj);
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

    return true;
}

extern "C" int scenecompiler_init(struct ce_api_a0 *api) {
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ct_renderer_a0);

    _G = (struct _G) {.allocator=ce_memory_a0->system};


    return 1;
}
