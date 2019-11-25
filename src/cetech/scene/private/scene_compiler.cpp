//==============================================================================
// Include
//==============================================================================

#include <celib/memory/allocator.h>
#include <celib/os/path.h>
#include <celib/yaml_cdb.h>
#include <celib/containers/array.h>
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include <celib/cdb.h>
#include <celib/config.h>

#include "cetech/machine/machine.h"
#include "cetech/resource/resource.h"
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include "cetech/ecs/ecs.h"
#include <cetech/scene/scene.h>
#include <cetech/asset_io/asset_io.h>

#define _G scene_compiler_globals

struct _G {
    ce_alloc_t0 *allocator;
} _G;

typedef char char_128[128];

typedef struct scene_compile_output_t {
    uint64_t *geom_name;
    uint32_t *ib_offset;
    uint32_t *vb_offset;
    bgfx_vertex_layout_t *vb_decl;
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
} scene_compile_output_t;

struct scene_compile_output_t *_crete_compile_output() {
    scene_compile_output_t *output = CE_ALLOC(_G.allocator,
                                              scene_compile_output_t,
                                              sizeof(scene_compile_output_t));
    *output = {};

    return output;
}

static void _destroy_compile_output(scene_compile_output_t *output) {
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

static bool _fill_scene(ce_cdb_t0 db, uint64_t obj, scene_compile_output_t* output) {
    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, obj);
    ce_cdb_a0->set_uint64(w, SCENE_GEOM_COUNT, ce_array_size(output->geom_name));
    ce_cdb_a0->set_uint64(w, SCENE_NODE_COUNT, ce_array_size(output->node_name));
    ce_cdb_a0->set_uint64(w, SCENE_IB_LEN, ce_array_size(output->ib));
    ce_cdb_a0->set_uint64(w, SCENE_VB_LEN, ce_array_size(output->vb));

    ce_cdb_a0->set_blob(w, SCENE_GEOM_NAME, output->geom_name,
                        sizeof(*output->geom_name) * ce_array_size(output->geom_name));

    ce_cdb_a0->set_blob(w, SCENE_IB_OFFSET, output->ib_offset,
                        sizeof(*output->ib_offset) * ce_array_size(output->ib_offset));

    ce_cdb_a0->set_blob(w, SCENE_VB_OFFSET, output->vb_offset,
                        sizeof(*output->vb_offset) * ce_array_size(output->vb_offset));

    ce_cdb_a0->set_blob(w, SCENE_VB_DECL, output->vb_decl,
                        sizeof(*output->vb_decl) * ce_array_size(output->vb_decl));

    ce_cdb_a0->set_blob(w, SCENE_IB_SIZE, output->ib_size,
                        sizeof(*output->ib_size) * ce_array_size(output->ib_size));

    ce_cdb_a0->set_blob(w, SCENE_VB_SIZE, output->vb_size,
                        sizeof(*output->vb_size) * ce_array_size(output->vb_size));

    ce_cdb_a0->set_blob(w, SCENE_IB_PROP, output->ib,
                        sizeof(*output->ib) * ce_array_size(output->ib));

    ce_cdb_a0->set_blob(w, SCENE_VB_PROP, output->vb,
                        sizeof(*output->vb) * ce_array_size(output->vb));

    ce_cdb_a0->set_blob(w, SCENE_NODE_NAME, output->node_name,
                        sizeof(*output->node_name) * ce_array_size(output->node_name));

    ce_cdb_a0->set_blob(w, SCENE_NODE_PARENT, output->node_parent,
                        sizeof(*output->node_parent) * ce_array_size(output->node_parent));

    ce_cdb_a0->set_blob(w, SCENE_NODE_POSE, output->node_pose,
                        sizeof(*output->node_pose) * ce_array_size(output->node_pose));

    ce_cdb_a0->set_blob(w, SCENE_NODE_GEOM, output->geom_node,
                        sizeof(*output->geom_node) * ce_array_size(output->geom_node));

    ce_cdb_a0->set_blob(w, SCENE_GEOM_STR, output->geom_str,
                        sizeof(*output->geom_str) * ce_array_size(output->geom_str));

    ce_cdb_a0->set_blob(w, SCENE_NODE_STR, output->node_str,
                        sizeof(*output->node_str) * ce_array_size(output->node_str));

    ce_cdb_a0->write_commit(w);

        return true;
}

#include "gltf_compiler.inl"

extern "C" int scenecompiler_init(struct ce_api_a0 *api) {
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_yaml_cdb_a0);
    CE_INIT_API(api, ce_yaml_cdb_a0);
    CE_INIT_API(api, ct_renderer_a0);

    _G = (struct _G) {
            .allocator=ce_memory_a0->system
    };

    api->add_impl(CT_DCC_ASSET_IO_I0_STR, &gltf_io, sizeof(gltf_io));

    ct_asset_io_a0->create_import_group(CT_SCENE_IMPORT_GROUP, 3.0f);

    return 1;
}
