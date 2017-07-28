#ifndef CETECH_SCENE_BLOB_H
#define CETECH_SCENE_BLOB_H

#include <stdint.h>
#include <bgfx/bgfx.h>

namespace scene_blob {

    typedef struct {
        uint32_t blob_version;

        uint32_t geom_count;
        uint32_t node_count;
        uint32_t ib_len;
        uint32_t vb_len;
        // uint64_t geom_name[geom_count];
        // uint32_t ib_offset[geom_count];
        // uint32_t vb_offset[geom_count];
        // bgfx::VertexDecl vb_decl[geom_count];
        // uint32_t ib_size[geom_count];
        // uint32_t vb_size[geom_count];
        // uint32_t ib[ib_len];
        // uint8_t vb[vb_len];
        // uint64_t node_name[node_count];
        // uint32_t node_parent[node_count];
        // float[16] node_pose[node_count];
        // uint64_t geom_node[geom_count];
    } blob_t;

    inline const blob_t* get(void* data) {
        return (blob_t*)(data);
    }

    inline uint32_t blob_size(const blob_t* blob) {
        return (
            sizeof(blob_t) +
            ( sizeof(uint64_t) * blob->geom_count ) +
            ( sizeof(uint32_t) * blob->geom_count ) +
            ( sizeof(uint32_t) * blob->geom_count ) +
            ( sizeof(bgfx::VertexDecl) * blob->geom_count ) +
            ( sizeof(uint32_t) * blob->geom_count ) +
            ( sizeof(uint32_t) * blob->geom_count ) +
            ( sizeof(uint32_t) * blob->ib_len ) +
            ( sizeof(uint8_t) * blob->vb_len ) +
            ( sizeof(uint64_t) * blob->node_count ) +
            ( sizeof(uint32_t) * blob->node_count ) +
            ( sizeof(float) * blob->node_count * 16) +
            ( sizeof(uint64_t) * blob->geom_count ) +
        0);
    }

    inline uint32_t geom_count(const blob_t* blob) {
        return blob->geom_count;
    }
    inline uint32_t node_count(const blob_t* blob) {
        return blob->node_count;
    }
    inline uint32_t ib_len(const blob_t* blob) {
        return blob->ib_len;
    }
    inline uint32_t vb_len(const blob_t* blob) {
        return blob->vb_len;
    }
    inline uint64_t* geom_name(const blob_t* blob) {
        return (uint64_t*)((blob) + 1);
    }
    inline uint32_t* ib_offset(const blob_t* blob) {
        return ((uint32_t*) (geom_name(blob) + (blob->geom_count)));
    }
    inline uint32_t* vb_offset(const blob_t* blob) {
        return ((uint32_t*) (ib_offset(blob) + (blob->geom_count)));
    }
    inline bgfx::VertexDecl* vb_decl(const blob_t* blob) {
        return ((bgfx::VertexDecl*) (vb_offset(blob) + (blob->geom_count)));
    }
    inline uint32_t* ib_size(const blob_t* blob) {
        return ((uint32_t*) (vb_decl(blob) + (blob->geom_count)));
    }
    inline uint32_t* vb_size(const blob_t* blob) {
        return ((uint32_t*) (ib_size(blob) + (blob->geom_count)));
    }
    inline uint32_t* ib(const blob_t* blob) {
        return ((uint32_t*) (vb_size(blob) + (blob->geom_count)));
    }
    inline uint8_t* vb(const blob_t* blob) {
        return ((uint8_t*) (ib(blob) + (blob->ib_len)));
    }
    inline uint64_t* node_name(const blob_t* blob) {
        return ((uint64_t*) (vb(blob) + (blob->vb_len)));
    }
    inline uint32_t* node_parent(const blob_t* blob) {
        return ((uint32_t*) (node_name(blob) + (blob->node_count)));
    }
    inline float* node_pose(const blob_t* blob) {
        return ((float*) (node_parent(blob) + (blob->node_count)));
    }
    inline uint64_t* geom_node(const blob_t* blob) {
        return ((uint64_t*) (node_pose(blob) + (blob->node_count*16)));
    }

}

#endif // CETECH_SCENE_BLOB_H