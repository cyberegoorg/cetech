#ifndef CETECH_SCENE_BLOB_H
#define CETECH_SCENE_BLOB_H

#include <celib/string/stringid.h>
#include <celib/math/types.h>

typedef struct scene_blob {
    u32 geom_count;
    u32 node_count;
    u32 ib_len;
    u32 vb_len;
    // stringid64_t geom_name[geom_count];
    // u32 ib_offset[geom_count];
    // u32 vb_offset[geom_count];
    // bgfx_vertex_decl_t vb_decl[geom_count];
    // u32 ib_size[geom_count];
    // u32 vb_size[geom_count];
    // u32 ib[ib_len];
    // u8 vb[vb_len];
    // stringid64_t node_name[node_count];
    // u32 node_parent[node_count];
    // cel_mat44f_t node_pose[node_count];
    // stringid64_t geom_node[geom_count];
} scene_blob_t;

#define scene_blob_geom_name(r) ((stringid64_t*) ((r) + 1))
#define scene_blob_ib_offset(r) ((u32*) (scene_blob_geom_name(r) + ((r)->geom_count)))
#define scene_blob_vb_offset(r) ((u32*) (scene_blob_ib_offset(r) + ((r)->geom_count)))
#define scene_blob_vb_decl(r) ((bgfx_vertex_decl_t*) (scene_blob_vb_offset(r) + ((r)->geom_count)))
#define scene_blob_ib_size(r) ((u32*) (scene_blob_vb_decl(r) + ((r)->geom_count)))
#define scene_blob_vb_size(r) ((u32*) (scene_blob_ib_size(r) + ((r)->geom_count)))
#define scene_blob_ib(r) ((u32*) (scene_blob_vb_size(r) + ((r)->geom_count)))
#define scene_blob_vb(r) ((u8*) (scene_blob_ib(r) + ((r)->ib_len)))
#define scene_blob_node_name(r) ((stringid64_t*) (scene_blob_vb(r) + ((r)->vb_len)))
#define scene_blob_node_parent(r) ((u32*) (scene_blob_node_name(r) + ((r)->node_count)))
#define scene_blob_node_pose(r) ((cel_mat44f_t*) (scene_blob_node_parent(r) + ((r)->node_count)))
#define scene_blob_geom_node(r) ((stringid64_t*) (scene_blob_node_pose(r) + ((r)->node_count)))

#endif // CETECH_SCENE_BLOB_H