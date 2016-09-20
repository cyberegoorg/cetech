#include <celib/containers/array.h>
#include <celib/yaml/yaml.h>
#include <celib/stringid/types.h>
#include <celib/containers/map.h>
#include <celib/stringid/stringid.h>

#include <engine/core/memory_system.h>
#include <engine/core/entcom.h>
#include <engine/renderer/material.h>
#include <bgfx/c99/bgfx.h>
#include <engine/components/transform.h>
#include <engine/renderer/mesh_renderer.h>
#include <engine/components/transform.h>

#include "../mesh_renderer.h"

ARRAY_PROTOTYPE(stringid64_t)

ARRAY_PROTOTYPE(material_t)


struct mesh_data {
    stringid64_t scene;
    stringid64_t mesh;
    stringid64_t node;
    stringid64_t material;
};

typedef struct {
    MAP_T(u32) ent_idx_map;

    ARRAY_T(stringid64_t) scene;
    ARRAY_T(stringid64_t) mesh;
    ARRAY_T(stringid64_t) node;

    ARRAY_T(material_t) material;
} world_data_t;

ARRAY_PROTOTYPE(world_data_t)

MAP_PROTOTYPE(world_data_t)

void _init_cubes();

#define _G meshGlobal
static struct G {
    stringid64_t type;

    MAP_T(world_data_t) world;
} _G = {0};


static void _new_world(world_t world) {
    world_data_t data = {0};

    MAP_INIT(u32, &data.ent_idx_map, memsys_main_allocator());

    ARRAY_INIT(stringid64_t, &data.scene, memsys_main_allocator());
    ARRAY_INIT(stringid64_t, &data.mesh, memsys_main_allocator());
    ARRAY_INIT(stringid64_t, &data.node, memsys_main_allocator());
    ARRAY_INIT(material_t, &data.material, memsys_main_allocator());

    MAP_SET(world_data_t, &_G.world, world.h.h, data);
}

static world_data_t *_get_world_data(world_t world) {
    return MAP_GET_PTR(world_data_t, &_G.world, world.h.h);
}

static void _destroy_world(world_t world) {
    world_data_t *data = _get_world_data(world);

    MAP_DESTROY(u32, &data->ent_idx_map);

    ARRAY_DESTROY(stringid64_t, &data->scene);
    ARRAY_DESTROY(stringid64_t, &data->mesh);
    ARRAY_DESTROY(stringid64_t, &data->node);
    ARRAY_DESTROY(material_t, &data->material);

}

int _mesh_component_compiler(yaml_node_t body,
                             ARRAY_T(u8) *data) {

    struct mesh_data t_data;

    char tmp_buffer[64] = {0};

    YAML_NODE_SCOPE(scene, body, "scene",
                    yaml_as_string(scene, tmp_buffer, CE_ARRAY_LEN(tmp_buffer));
                            t_data.scene = stringid64_from_string(tmp_buffer);
    );
    YAML_NODE_SCOPE(mesh, body, "mesh",
                    yaml_as_string(mesh, tmp_buffer, CE_ARRAY_LEN(tmp_buffer));
                            t_data.mesh = stringid64_from_string(tmp_buffer);
    );

    YAML_NODE_SCOPE(material, body, "material",
                    yaml_as_string(material, tmp_buffer, CE_ARRAY_LEN(tmp_buffer));
                            t_data.material = stringid64_from_string(tmp_buffer);
    );

    YAML_NODE_SCOPE(node, body, "node",
                    if (yaml_is_valid(node)) {
                        yaml_as_string(node, tmp_buffer, CE_ARRAY_LEN(tmp_buffer));
                        t_data.node = stringid64_from_string(tmp_buffer);
                    }
    );

    ARRAY_PUSH(u8, data, (u8 *) &t_data, sizeof(t_data));

    return 1;
}

static void _on_world_create(world_t world) {
    _new_world(world);
}

static void _on_world_destroy(world_t world) {
    _destroy_world(world);
}

static void _destroyer(world_t world,
                       entity_t *ents,
                       size_t ent_count) {
    world_data_t *world_data = _get_world_data(world);

    // TODO: remove from arrays, swap idx -> last AND change size
    for (int i = 0; i < ent_count; i++) {
        MAP_REMOVE(u32, &world_data->ent_idx_map, ents[i].idx);
    }
}

static void _spawner(world_t world,
                     entity_t *ents,
                     entity_t *ents_parent,
                     size_t ent_count,
                     void *data) {
    struct mesh_data *tdata = data;

    _init_cubes();

    for (int i = 0; i < ent_count; ++i) {
        mesh_create(world,
                    ents[i],
                    tdata->scene,
                    tdata->mesh,
                    tdata->node,
                    tdata->material);
    }
}


static const uint32_t packUint32(uint8_t _x,
                                 uint8_t _y,
                                 uint8_t _z,
                                 uint8_t _w) {
    union {
        uint32_t ui32;
        uint8_t arr[4];
    } un;

    un.arr[0] = _x;
    un.arr[1] = _y;
    un.arr[2] = _z;
    un.arr[3] = _w;

    return un.ui32;
}

static const uint32_t packF4u(float _x,
                              float _y,
                              float _z,
                              float _w) {
    const uint8_t xx = (uint8_t) (_x * 127.0f + 128.0f);
    const uint8_t yy = (uint8_t) (_y * 127.0f + 128.0f);
    const uint8_t zz = (uint8_t) (_z * 127.0f + 128.0f);
    const uint8_t ww = (uint8_t) (_w * 127.0f + 128.0f);

    return packUint32(xx, yy, zz, ww);
}

typedef struct {
    float m_x;
    float m_y;
    float m_z;
    uint32_t m_normal;
    uint32_t m_tangent;
    int16_t m_u;
    int16_t m_v;
} PosNormalTangentTexcoordVertex;

static const uint16_t s_cubeIndices[36] =
        {
                0, 2, 1,
                1, 2, 3,
                4, 5, 6,
                5, 7, 6,

                8, 10, 9,
                9, 10, 11,
                12, 13, 14,
                13, 15, 14,

                16, 18, 17,
                17, 18, 19,
                20, 21, 22,
                21, 23, 22,
        };

PosNormalTangentTexcoordVertex s_cubeVertices[24];
bgfx_vertex_decl_t vertex_decl = {0};

bgfx_vertex_buffer_handle_t vbh;
bgfx_index_buffer_handle_t ibh;

static int init = 0;

void _init_cubes() {
    if (init) {
        return;
    }
    init = 1;

    PosNormalTangentTexcoordVertex v[24] = {
            {-1.0f, 1.0f,  1.0f,  packF4u(0.0f, 0.0f, 1.0f, 0.0f),  0, 0,      0},
            {1.0f,  1.0f,  1.0f,  packF4u(0.0f, 0.0f, 1.0f, 0.0f),  0, 0x7fff, 0},
            {-1.0f, -1.0f, 1.0f,  packF4u(0.0f, 0.0f, 1.0f, 0.0f),  0, 0,      0x7fff},
            {1.0f,  -1.0f, 1.0f,  packF4u(0.0f, 0.0f, 1.0f, 0.0f),  0, 0x7fff, 0x7fff},
            {-1.0f, 1.0f,  -1.0f, packF4u(0.0f, 0.0f, -1.0f, 0.0f), 0, 0,      0},
            {1.0f,  1.0f,  -1.0f, packF4u(0.0f, 0.0f, -1.0f, 0.0f), 0, 0x7fff, 0},
            {-1.0f, -1.0f, -1.0f, packF4u(0.0f, 0.0f, -1.0f, 0.0f), 0, 0,      0x7fff},
            {1.0f,  -1.0f, -1.0f, packF4u(0.0f, 0.0f, -1.0f, 0.0f), 0, 0x7fff, 0x7fff},
            {-1.0f, 1.0f,  1.0f,  packF4u(0.0f, 1.0f, 0.0f, 0.0f),  0, 0,      0},
            {1.0f,  1.0f,  1.0f,  packF4u(0.0f, 1.0f, 0.0f, 0.0f),  0, 0x7fff, 0},
            {-1.0f, 1.0f,  -1.0f, packF4u(0.0f, 1.0f, 0.0f, 0.0f),  0, 0,      0x7fff},
            {1.0f,  1.0f,  -1.0f, packF4u(0.0f, 1.0f, 0.0f, 0.0f),  0, 0x7fff, 0x7fff},
            {-1.0f, -1.0f, 1.0f,  packF4u(0.0f, -1.0f, 0.0f, 0.0f), 0, 0,      0},
            {1.0f,  -1.0f, 1.0f,  packF4u(0.0f, -1.0f, 0.0f, 0.0f), 0, 0x7fff, 0},
            {-1.0f, -1.0f, -1.0f, packF4u(0.0f, -1.0f, 0.0f, 0.0f), 0, 0,      0x7fff},
            {1.0f,  -1.0f, -1.0f, packF4u(0.0f, -1.0f, 0.0f, 0.0f), 0, 0x7fff, 0x7fff},
            {1.0f,  -1.0f, 1.0f,  packF4u(1.0f, 0.0f, 0.0f, 0.0f),  0, 0,      0},
            {1.0f,  1.0f,  1.0f,  packF4u(1.0f, 0.0f, 0.0f, 0.0f),  0, 0x7fff, 0},
            {1.0f,  -1.0f, -1.0f, packF4u(1.0f, 0.0f, 0.0f, 0.0f),  0, 0,      0x7fff},
            {1.0f,  1.0f,  -1.0f, packF4u(1.0f, 0.0f, 0.0f, 0.0f),  0, 0x7fff, 0x7fff},
            {-1.0f, -1.0f, 1.0f,  packF4u(-1.0f, 0.0f, 0.0f, 0.0f), 0, 0,      0},
            {-1.0f, 1.0f,  1.0f,  packF4u(-1.0f, 0.0f, 0.0f, 0.0f), 0, 0x7fff, 0},
            {-1.0f, -1.0f, -1.0f, packF4u(-1.0f, 0.0f, 0.0f, 0.0f), 0, 0,      0x7fff},
            {-1.0f, 1.0f,  -1.0f, packF4u(-1.0f, 0.0f, 0.0f, 0.0f), 0, 0x7fff, 0x7fff},
    };

    memory_copy(s_cubeVertices, v, 24 * sizeof(PosNormalTangentTexcoordVertex));

    bgfx_vertex_decl_begin(&vertex_decl, BGFX_RENDERER_TYPE_NULL);

    bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_POSITION, 3, BGFX_ATTRIB_TYPE_FLOAT, 0, 0);
    bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_NORMAL, 4, BGFX_ATTRIB_TYPE_UINT8, 1, 1);
    bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_TANGENT, 4, BGFX_ATTRIB_TYPE_UINT8, 1, 1);
    bgfx_vertex_decl_add(&vertex_decl, BGFX_ATTRIB_TEXCOORD0, 2, BGFX_ATTRIB_TYPE_INT16, 1, 1);

    bgfx_vertex_decl_end(&vertex_decl);

    vbh = bgfx_create_vertex_buffer(bgfx_make_ref(s_cubeVertices, sizeof(s_cubeVertices)), &vertex_decl, 0);
    ibh = bgfx_create_index_buffer(bgfx_make_ref(s_cubeIndices, sizeof(s_cubeVertices)), 0);
}


int mesh_init(int stage) {
    if (stage == 0) {
        _G = (struct G) {0};

        return 1;
    }

    MAP_INIT(world_data_t, &_G.world, memsys_main_allocator());

    _G.type = stringid64_from_string("mesh_renderer");

    component_register_compiler(_G.type, _mesh_component_compiler, 10);
    component_register_type(_G.type, _spawner, _destroyer, _on_world_create, _on_world_destroy);

    return 1;
}

void mesh_shutdown() {

    MAP_DESTROY(world_data_t, &_G.world);

    _G = (struct G) {0};
}

int mesh_is_valid(mesh_t mesh) {
    return mesh.idx != UINT32_MAX;
}

int mesh_has(world_t world,
             entity_t entity) {
    world_data_t *world_data = _get_world_data(world);
    return MAP_HAS(u32, &world_data->ent_idx_map, entity.h.h);
}

mesh_t mesh_get(world_t world,
                entity_t entity) {

    world_data_t *world_data = _get_world_data(world);
    u32 idx = MAP_GET(u32, &world_data->ent_idx_map, entity.h.h, UINT32_MAX);
    return (mesh_t) {.idx = idx};
}

mesh_t mesh_create(world_t world,
                   entity_t entity,
                   stringid64_t scene,
                   stringid64_t mesh,
                   stringid64_t node,
                   stringid64_t material) {

    world_data_t *data = _get_world_data(world);

    log_debug("mesh_renderer", "create mesh renderer");

    material_t material_instance = material_resource_create(material);

    u32 idx = (u32) ARRAY_SIZE(&data->material);

    MAP_SET(u32, &data->ent_idx_map, entity.h.h, idx);

    ARRAY_PUSH_BACK(stringid64_t, &data->scene, scene);
    ARRAY_PUSH_BACK(stringid64_t, &data->mesh, mesh);
    ARRAY_PUSH_BACK(stringid64_t, &data->node, node);
    ARRAY_PUSH_BACK(material_t, &data->material, material_instance);

    return (mesh_t) {.idx = idx};
}

void mesh_render_all(world_t world) {
    world_data_t *data = _get_world_data(world);

    const MAP_ENTRY_T(u32) *ce_it = MAP_BEGIN(u32, &data->ent_idx_map);
    const MAP_ENTRY_T(u32) *ce_end = MAP_END(u32, &data->ent_idx_map);
    while (ce_it != ce_end) {
        material_t material = ARRAY_AT(&data->material, ce_it->value);

        material_use(material);

        transform_t t = transform_get(world, (entity_t) {.idx = ce_it->key});
        mat44f_t *w = transform_get_world_matrix(world, t);

        bgfx_set_transform(w, 1);

        bgfx_set_vertex_buffer(vbh, 0, 24);
        bgfx_set_index_buffer(ibh, 0, 36);

        material_submit(material);

        ++ce_it;
    }
}

material_t mesh_get_material(world_t world,
                             mesh_t mesh) {
    world_data_t *data = _get_world_data(world);

    return ARRAY_AT(&data->material, mesh.idx);
}