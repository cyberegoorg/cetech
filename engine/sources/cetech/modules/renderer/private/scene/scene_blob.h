#ifndef CETECH_SCENE_BLOB_H
#define CETECH_SCENE_BLOB_H

#include <cetech/core/hash.h>
#include <cetech/core/math_types.h>

typedef struct scene_blob {
    uint32_t geom_count;
    uint32_t node_count;
    uint32_t ib_len;
    uint32_t vb_len;
    // stringid64_t geom_name[geom_count];
    // uint32_t ib_offset[geom_count];
    // uint32_t vb_offset[geom_count];
    // bgfx_vertex_decl_t vb_decl[geom_count];
    // uint32_t ib_size[geom_count];
    // uint32_t vb_size[geom_count];
    // uint32_t ib[ib_len];
    // uint8_t vb[vb_len];
    // stringid64_t node_name[node_count];
    // uint32_t node_parent[node_count];
    // mat44f_t node_pose[node_count];
    // stringid64_t geom_node[geom_count];
} scene_blob_t;

#define scene_blob_geom_name(r) ((stringid64_t*) ((r) + 1))
#define scene_blob_ib_offset(r) ((uint32_t*) (scene_blob_geom_name(r) + ((r)->geom_count)))
#define scene_blob_vb_offset(r) ((uint32_t*) (scene_blob_ib_offset(r) + ((r)->geom_count)))
#define scene_blob_vb_decl(r) ((bgfx_vertex_decl_t*) (scene_blob_vb_offset(r) + ((r)->geom_count)))
#define scene_blob_ib_size(r) ((uint32_t*) (scene_blob_vb_decl(r) + ((r)->geom_count)))
#define scene_blob_vb_size(r) ((uint32_t*) (scene_blob_ib_size(r) + ((r)->geom_count)))
#define scene_blob_ib(r) ((uint32_t*) (scene_blob_vb_size(r) + ((r)->geom_count)))
#define scene_blob_vb(r) ((uint8_t*) (scene_blob_ib(r) + ((r)->ib_len)))
#define scene_blob_node_name(r) ((stringid64_t*) (scene_blob_vb(r) + ((r)->vb_len)))
#define scene_blob_node_parent(r) ((uint32_t*) (scene_blob_node_name(r) + ((r)->node_count)))
#define scene_blob_node_pose(r) ((mat44f_t*) (scene_blob_node_parent(r) + ((r)->node_count)))
#define scene_blob_geom_node(r) ((stringid64_t*) (scene_blob_node_pose(r) + ((r)->node_count)))

#endif // CETECH_SCENE_BLOB_H