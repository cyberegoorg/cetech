#include <string.h>
#include <stdlib.h>

#include "celib/hashlib.h"
#include "celib/config.h"
#include "celib/memory.h"
#include <celib/module.h>
#include <celib/fmath.inl>
#include <celib/macros.h>
#include <celib/log.h>

#include "celib/api_system.h"
#include "cetech/resource/resource.h"
#include "cetech/ecs/ecs.h"
#include "cetech/transform/transform.h"
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/material/material.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/editor/property.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/mesh/primitive_mesh.h>
#include <cetech/mesh/mesh_renderer.h>

#define LOG_WHERE "primitive_mesh"

#define _G primitice_mesh_global

static struct _G {
    struct ce_alloc *allocator;
} _G;


struct mesh_render_data {
    uint8_t viewid;
    uint64_t layer_name;
};


struct pt_vertex {
    float m_x;
    float m_y;
    float m_z;
    float m_u;
    float m_v;
};
static bgfx_vertex_decl_t pt_vertex_decl;

static const struct pt_vertex _cube_vertices[24] = {
        // Back
        {-1, -1, 1,  1, 1},
        {1,  1,  1,  0, 0},
        {-1, 1,  1,  1, 0},
        {1,  -1, 1,  0, 1},

        // Front
        {-1, -1, -1, 0, 1},
        {1,  1,  -1, 1, 0},
        {-1, 1,  -1, 0, 0},
        {1,  -1, -1, 1, 1},

        // Right
        {1,  -1, -1, 0, 1},
        {1,  1,  1,  1, 0},
        {1,  -1, 1,  1, 1},
        {1,  1,  -1, 0, 0},

        // Left
        {-1, -1, -1, 1, 1},
        {-1, 1,  1,  0, 0},
        {-1, -1, 1,  0, 1},
        {-1, 1,  -1, 1, 0},

        // Top
        {-1, 1,  -1, 0, 1},
        {1,  1,  1,  1, 0},
        {-1, 1,  1,  0, 0},
        {1,  1,  -1, 1, 1},

        // Bottom
        {-1, -1, -1, 0, 0},
        {1,  -1, 1,  1, 1},
        {-1, -1, 1,  0, 1},
        {1,  -1, -1, 1, 0},
};

static const uint16_t cube_indices[36] = {
                0, 1, 2,
                0, 3, 1,
                4, 6, 5,
                4, 5, 7,

                8, 9, 10,
                8, 11, 9,
                12, 14, 13,
                12, 13, 15,

                16, 18, 17,
                16, 17, 19,
                20, 21, 22,
                20, 23, 21,
        };

static bgfx_vertex_buffer_handle_t cube_vbh;
static bgfx_index_buffer_handle_t cube_ibh;

void foreach_primitive_mesh(struct ct_world world,
                            struct ct_entity *entities,
                            ct_entity_storage_t *item,
                            uint32_t n,
                            void *_data) {
    struct mesh_render_data *data = _data;

    struct ct_transform_comp *transforms = ct_ecs_a0->get_all(
            TRANSFORM_COMPONENT, item);

    struct ct_primitive_mesh *primitives = ct_ecs_a0->get_all(
            PRIMITIVE_MESH_COMPONENT,
            item);

    for (int i = 0; i < n; ++i) {
        struct ct_transform_comp *tr = &transforms[i];
        struct ct_primitive_mesh *m = &primitives[i];

        if (!m->material) {
            continue;
        }

        float final_w[16];
        ce_mat4_identity(final_w);
        ce_mat4_move(final_w, tr->world);

        ct_gfx_a0->bgfx_set_transform(&final_w, 1);
        ct_gfx_a0->bgfx_set_vertex_buffer(0, cube_vbh, 0,
                                          CE_ARRAY_LEN(_cube_vertices));
        ct_gfx_a0->bgfx_set_index_buffer(cube_ibh, 0, CE_ARRAY_LEN(cube_indices));

        uint64_t material_obj = m->material;

        ct_material_a0->submit(material_obj, data->layer_name, data->viewid);
    }
}

static void _init_api(struct ce_api_a0 *api) {
}

static uint64_t cdb_type() {
    return PRIMITIVE_MESH_COMPONENT;
}

static const char *display_name() {
    return ICON_FA_HOUZZ " Primitive mesh";
}

static void property_editor(uint64_t obj) {
    ct_editor_ui_a0->prop_resource(obj,
                                   ce_id_a0->id64("material"), "Material",
                                   ce_id_a0->id64("material"), obj + 1);

}


static struct ct_property_editor_i0 ct_property_editor_i0 = {
        .cdb_type = cdb_type,
        .draw_ui = property_editor,
};

static void render(struct ct_world world,
                   struct ct_rg_builder *builder) {

    uint8_t viewid = builder->get_layer_viewid(builder, _GBUFFER);


    struct mesh_render_data render_data = {
            .viewid = viewid,
            .layer_name = _GBUFFER,
    };

    ct_ecs_a0->process(world,
                       ct_ecs_a0->mask(PRIMITIVE_MESH_COMPONENT) |
                       ct_ecs_a0->mask(TRANSFORM_COMPONENT),
                       foreach_primitive_mesh, &render_data);
}

static struct ct_renderer_component_i0 ct_renderer_component_i = {
        .render = render
};

static void *get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = display_name,
        };

        return &ct_editor_component_i0;
    } else if (CT_RENDERER_COMPONENT_I == name_hash) {
        return &ct_renderer_component_i;
    }

    return NULL;
}

static uint64_t size() {
    return sizeof(struct ct_primitive_mesh);
}

static void mesh_spawner(struct ct_world world,
                         uint64_t obj,
                         void *data) {
    const ce_cdb_obj_o *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    struct ct_primitive_mesh *m = data;

    *m = (struct ct_primitive_mesh) {
            .material = ce_cdb_a0->read_ref(r, PROP_MATERIAL, 0),
    };

}


static struct ct_component_i0 ct_component_i0 = {
        .cdb_type = cdb_type,
        .size = size,
        .get_interface = get_interface,
        .spawner = mesh_spawner,
};

static void _init(struct ce_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->register_api(COMPONENT_INTERFACE, &ct_component_i0);
    api->register_api(PROPERTY_EDITOR_INTERFACE, &ct_property_editor_i0);

    ct_gfx_a0->bgfx_vertex_decl_begin(&pt_vertex_decl,
                                      ct_gfx_a0->bgfx_get_renderer_type());

    ct_gfx_a0->bgfx_vertex_decl_add(&pt_vertex_decl,
                                    BGFX_ATTRIB_POSITION, 3,
                                    BGFX_ATTRIB_TYPE_FLOAT, false, false);

    ct_gfx_a0->bgfx_vertex_decl_add(&pt_vertex_decl,
                                    BGFX_ATTRIB_TEXCOORD0, 2,
                                    BGFX_ATTRIB_TYPE_FLOAT, false, false);

    ct_gfx_a0->bgfx_vertex_decl_end(&pt_vertex_decl);

    cube_vbh = ct_gfx_a0->bgfx_create_vertex_buffer(
            ct_gfx_a0->bgfx_make_ref(_cube_vertices, sizeof(_cube_vertices)),
            &pt_vertex_decl, BGFX_BUFFER_NONE);

    cube_ibh = ct_gfx_a0->bgfx_create_index_buffer(
            ct_gfx_a0->bgfx_make_ref(cube_indices, sizeof(cube_indices)),
            BGFX_BUFFER_NONE);
}

static void _shutdown() {
}

static void init(struct ce_api_a0 *api) {
    _init(api);
}

static void shutdown() {
    _shutdown();
}


CE_MODULE_DEF(
        primitice_mesh,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_material_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ct_renderer_a0);

        },
        {
            CE_UNUSED(reload);
            init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);

            shutdown();
        }
)