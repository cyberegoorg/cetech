extern "C" {
#include <celib/bounds.h>
#include "celib/api_system.h"
#include "celib/module.h"
#include "celib/memory.h"
#include <celib/hashlib.h>
#include <celib/array.inl>
#include <celib/log.h>

#include <cetech/gfx/renderer.h>
#include <cetech/gfx/debugdraw.h>
}

#include <cetech/gfx/private/debugdraw/debugdraw.h>


#define _G DebugDrawGLobal

static struct _G {
    ce_alloc *allocator;
} _G;


static ct_dd_sprite _ddCreateSprite(uint16_t _width,
                                    uint16_t _height,
                                    const void *_data) {
    return {.idx = ddCreateSprite(_width, _height, _data).idx};
}

static void _ddDestroySprite(ct_dd_sprite _handle) {
    SpriteHandle sh = {.idx=_handle.idx};
    ddDestroy(sh);
}

static ct_dd_geometry _ddCreateGeometry(uint32_t _numVertices,
                                        const ct_dd_vertex *_vertices,
                                        uint32_t _numIndices = 0,
                                        const uint16_t *_indices = NULL) {
    return {.idx=ddCreateGeometry(
            _numVertices,
            reinterpret_cast<const DdVertex *>(_vertices),
            _numIndices,
            _indices).idx};
}


static void _ddDestroyGeometry(ct_dd_geometry _handle) {
    GeometryHandle gh = {.idx=_handle.idx};
    ddDestroy(gh);
}

void _ddDraw_aabb(const struct ce_aabb _aabb) {
    Aabb *aabb = (Aabb *) &_aabb;
    ddDraw(*aabb);
};


void _ddDraw_cylinder(const struct ce_cylinder _cylinder) {
    Cylinder *tmp = (Cylinder *) &_cylinder;
    ddDraw(*tmp);
}


void _ddDraw_capsule(const struct ce_capsule _capsule) {
    Capsule *tmp = (Capsule *) &_capsule;
    ddDraw(*tmp);
}


void _ddDraw_disk(const struct ce_disk _disk) {
    Disk *tmp = (Disk *) &_disk;
    ddDraw(*tmp);
}


void _ddDraw_obb(const struct ce_obb _obb) {
    Obb *tmp = (Obb *) &_obb;
    ddDraw(*tmp);
}


void _ddDraw_sphere(const struct ce_sphere _sphere) {
    Sphere *tmp = (Sphere *) &_sphere;
    ddDraw(*tmp);
}


void _ddDraw_cone(const struct ce_cone _cone) {
    Cone *tmp = (Cone *) &_cone;
    ddDraw(*tmp);
}


void _ddDraw_handle(ct_dd_geometry _handle) {
    GeometryHandle gh = {.idx = _handle.idx};
    ddDraw(gh);
}

void _ddDrawArc(ct_dd_axis _axis,
                float _x,
                float _y,
                float _z,
                float _radius,
                float _degrees) {
    Axis::Enum a = static_cast<Axis::Enum>(_axis);

    ddDrawArc(a,
              _x,
              _y,
              _z,
              _radius,
              _degrees);
}

void _ddDrawCircle_axis(ct_dd_axis _axis,
                        float _x,
                        float _y,
                        float _z,
                        float _radius,
                        float _weight = 0.0f) {
    Axis::Enum a = static_cast<Axis::Enum>(_axis);

    ddDrawCircle(a,
                 _x,
                 _y,
                 _z,
                 _radius,
                 _weight);
}


void _ddDrawQuad_sprite(ct_dd_sprite _handle,
                        const float *_normal,
                        const float *_center,
                        float _size) {
    SpriteHandle sh = {.idx = _handle.idx};

    ddDrawQuad(sh,
               _normal,
               _center,
               _size);
}


void _ddDrawQuad_texture(ct_render_texture_handle _handle,
                         const float *_normal,
                         const float *_center,
                         float _size) {

    bgfx::TextureHandle th = {.idx = _handle.idx};

    ddDrawQuad(th,
               _normal,
               _center,
               _size);
}


void _ddDrawAxis(float _x,
                 float _y,
                 float _z,
                 float _len,
                 ct_dd_axis _highlight,
                 float _thickness) {
    Axis::Enum a = static_cast<Axis::Enum>(_highlight);

    ddDrawAxis(_x,
               _y,
               _z,
               _len,
               a,
               _thickness);
}


void _ddDrawGrid_axis(ct_dd_axis _axis,
                      const void *_center,
                      uint32_t _size = 20,
                      float _step = 1.0f) {
    Axis::Enum a = static_cast<Axis::Enum>(_axis);
    ddDrawGrid(a, _center, _size, _step);
}


void _ddDrawOrb(float _x,
                float _y,
                float _z,
                float _radius,
                ct_dd_axis _highlight) {
    ddDrawOrb(_x,
              _y,
              _z,
              _radius,
              static_cast<Axis::Enum>(_highlight));
}

void _ddDrawLineList(uint32_t _numVertices,
                     const ct_dd_vertex *_vertices,
                     uint32_t _numIndices = 0,
                     const uint16_t *_indices = NULL) {
    ddDrawLineList(_numVertices,
                   reinterpret_cast<const DdVertex *>(_vertices),
                   _numIndices,
                   _indices);
}


void _ddDrawTriList(uint32_t _numVertices,
                    const ct_dd_vertex *_vertices,
                    uint32_t _numIndices = 0,
                    const uint16_t *_indices = NULL) {
    ddDrawTriList(_numVertices,
                  reinterpret_cast<const DdVertex *>(_vertices),
                  _numIndices,
                  _indices);
}


static struct ct_dd_a0 debugdraw_api = {
        .begin = ddBegin,
        .end = ddEnd,
        .push = ddPush,
        .pop = ddPop,
        .set_state = ddSetState,
        .set_color = ddSetColor,
        .set_lod = ddSetLod,
        .set_wireframe = ddSetWireframe,
        .set_stipple = ddSetStipple,
        .set_spin = ddSetSpin,
        .set_transform_mtx = ddSetTransform,
        .set_translate = ddSetTranslate,
        .move_to = ddMoveTo,
        .move_to_pos = ddMoveTo,
        .line_to = ddLineTo,
        .line_to_pos = ddLineTo,
        .close = ddClose,
        .draw_frustum = ddDrawFrustum,
        .draw_circle = ddDrawCircle,
        .draw_quad = ddDrawQuad,
        .draw_cone2 = ddDrawCone,
        .draw_cylinder2 = ddDrawCylinder,
        .draw_capsule2 = ddDrawCapsule,
        .draw_grid = ddDrawGrid,

        .create_sprite = _ddCreateSprite,
        .destroy_sprite = _ddDestroySprite,
        .create_geometry = _ddCreateGeometry,
        .destroy_geometry = _ddDestroyGeometry,
        .draw_aabb = _ddDraw_aabb,
        .draw_cylinder = _ddDraw_cylinder,
        .draw_capsule = _ddDraw_capsule,
        .draw_disk = _ddDraw_disk,
        .draw_obb = _ddDraw_obb,
        .draw_sphere = _ddDraw_sphere,
        .draw_cone = _ddDraw_cone,
        .draw_geometry = _ddDraw_handle,
        .draw_line_list = _ddDrawLineList,
        .draw_tri_list = _ddDrawTriList,
        .draw_arc = _ddDrawArc,
        .draw_axis = _ddDrawAxis,
        .draw_circle_axis = _ddDrawCircle_axis,
        .draw_quad_sprite = _ddDrawQuad_sprite,
        .draw_quad_texture = _ddDrawQuad_texture,
        .draw_grid_axis = _ddDrawGrid_axis,
        .draw_orb = _ddDrawOrb,
};

struct ct_dd_a0 *ct_dd_a0 = &debugdraw_api;


static void _init(struct ce_api_a0 *api) {
    api->register_api("ct_dd_a0", &debugdraw_api);

    _G = (struct _G){
            .allocator = ce_memory_a0->system
    };

    ddInit();

}

static void _shutdown() {
    ddShutdown();

    _G = (struct _G){};
}

CE_MODULE_DEF(
        debugdraw,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_log_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)
