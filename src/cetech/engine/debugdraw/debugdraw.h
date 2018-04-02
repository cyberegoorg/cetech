#ifndef CETECH_DEBUDRAW_H
#define CETECH_DEBUDRAW_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdbool.h>

struct ct_aabb;
struct ct_cylinder;
struct ct_capsule;
struct ct_disk;
struct ct_obb;
struct ct_sphere;
struct ct_cone;

//==============================================================================
// Enums
//==============================================================================

enum ct_dd_axis {
    DD_X,
    DD_Y,
    DD_Z,
    DD_AXIS_COUNT
};


//==============================================================================
// Structs
//==============================================================================

struct ct_dd_vertex {
    float x, y, z;
};

struct ct_dd_sprite {
    uint16_t idx;
};

struct ct_dd_geometry {
    uint16_t idx;
};


//==============================================================================
// Debugdraw
//==============================================================================

struct ct_dd_a0 {
    struct ct_dd_sprite (*create_sprite)(uint16_t width,
                                         uint16_t height,
                                         const void *data);

    void (*destroy_sprite)(struct ct_dd_sprite handle);

    struct ct_dd_geometry (*create_geometry)(uint32_t numVertices,
                                             const struct ct_dd_vertex *vertices,
                                             uint32_t numIndices/* = 0*/,
                                             const uint16_t *indices/* = NULL*/);


    void (*destroy_geometry)(struct ct_dd_geometry _handle);


    void (*begin)(uint16_t _viewId);


    void (*end)();


    void (*push)();


    void (*pop)();


    void (*set_state)(bool depthTest,
                      bool depthWrite,
                      bool clockwise);


    void (*set_color)(uint32_t abgr);


    void (*set_lod)(uint8_t lod);


    void (*set_wireframe)(bool wireframe);


    void (*set_stipple)(bool stipple,
                        float scale/* = 1.0f*/,
                        float offset/* = 0.0f*/);


    void (*set_spin)(float spin);


    void (*set_transform_mtx)(const void *mtx);


    void (*set_translate)(float x,
                          float y,
                          float z);


    void (*move_to)(float x,
                    float y,
                    float z /*= 0.0f*/);


    void (*move_to_pos)(const void *pos);


    void (*line_to)(float x,
                    float y,
                    float z/* = 0.0f*/);


    void (*line_to_pos)(const void *pos);


    void (*close)();


    void (*draw_aabb)(const struct ct_aabb aabb);


    void (*draw_cylinder)(const struct ct_cylinder cylinder);


    void (*draw_capsule)(const struct ct_capsule capsule);


    void (*draw_disk)(const struct ct_disk disk);


    void (*draw_obb)(const struct ct_obb obb);


    void (*draw_sphere)(const struct ct_sphere sphere);


    void (*draw_cone)(const struct ct_cone cone);


    void (*draw_geometry)(struct ct_dd_geometry handle);


    void (*draw_line_list)(uint32_t numVertices,
                           const struct ct_dd_vertex *vertices,
                           uint32_t numIndices /*= 0*/,
                           const uint16_t *indices /*= NULL*/);


    void (*draw_tri_list)(uint32_t numVertices,
                          const struct ct_dd_vertex *vertices,
                          uint32_t numIndices/* = 0*/,
                          const uint16_t *_indices /*= NULL*/);


    void (*draw_frustum)(const void *viewProj);


    void (*draw_arc)(enum ct_dd_axis dd_axis,
                     float x,
                     float y,
                     float z,
                     float radius,
                     float degrees);


    void (*draw_circle)(const void *normal,
                        const void *center,
                        float radius,
                        float weight /*= 0.0f*/);


    void (*draw_circle_axis)(enum ct_dd_axis dd_axis,
                             float x,
                             float y,
                             float z,
                             float radius,
                             float weight/* = 0.0f*/);


    void (*draw_quad)(const float *normal,
                      const float *center,
                      float size);


    void (*draw_quad_sprite)(struct ct_dd_sprite handle,
                             const float *normal,
                             const float *center,
                             float size);


    void (*draw_quad_texture)(struct ct_texture handle,
                              const float *normal,
                              const float *center,
                              float ßsize);


    void (*draw_cone2)(const void *from,
                       const void *to,
                       float radius);


    void (*draw_cylinder2)(const void *from,
                           const void *to,
                           float radius);


    void (*draw_capsule2)(const void *from,
                          const void *to,
                          float radius);


    void (*draw_axis)(float x,
                      float y,
                      float z,
                      float len /*= 1.0f*/,
                      enum ct_dd_axis highlight,
                      float thickness /*= 0.0f*/);


    void (*draw_grid)(const void *normal,
                      const void *center,
                      uint32_t size/* = 20*/,
                      float step /*= 1.0f*/);


    void (*draw_grid_axis)(enum ct_dd_axis dd_axis,
                           const void *center,
                           uint32_t size /*= 20*/,
                           float step /*= 1.0f*/);


    void (*draw_orb)(float x,
                     float y,
                     float z,
                     float radius,
                     enum ct_dd_axis highlight);
};

#ifdef __cplusplus
}
#endif

#endif // CETECH_DEBUDRAW_H