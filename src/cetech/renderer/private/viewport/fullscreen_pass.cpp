//==============================================================================
// includes
//==============================================================================
#include <cstdio>

#include <celib/allocator.h>
#include <celib/array.inl>
#include <celib/map.inl>
#include <celib/handler.inl>
#include <celib/fpumath.h>

#include <cetech/yaml/yaml.h>
#include <cetech/api/api_system.h>
#include <cetech/config/config.h>
#include <cetech/macros.h>
#include <cetech/module/module.h>
#include <cetech/hashlib/hashlib.h>

#include <cetech/entity/entity.h>

#include <cetech/renderer/renderer.h>
#include <cetech/renderer/texture.h>
#include <cetech/camera/camera.h>
#include <cetech/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/debugui/debugui.h>
#include <cetech/renderer/mesh_renderer.h>
#include <cetech/blob/blob.h>


#include "bgfx/platform.h"

#include <cetech/renderer/viewport.h>
#include "cetech/renderer/scene.h"
#include "cetech/renderer/material.h"

CETECH_DECL_API(ct_viewport_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_material_a0);


//==============================================================================
// GLobals
//==============================================================================

struct PosTexCoord0Vertex {
    float m_x;
    float m_y;
    float m_z;
    float m_u;
    float m_v;

    static void init() {
        ms_decl.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .end();
    }

    static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosTexCoord0Vertex::ms_decl;


void screenspace_quad(float _textureWidth,
                      float _textureHeight,
                      float _texelHalf,
                      bool _originBottomLeft,
                      float _width = 1.0f,
                      float _height = 1.0f) {
    if (3 ==
        bgfx::getAvailTransientVertexBuffer(3, PosTexCoord0Vertex::ms_decl)) {
        bgfx::TransientVertexBuffer vb;
        bgfx::allocTransientVertexBuffer(&vb, 3, PosTexCoord0Vertex::ms_decl);
        PosTexCoord0Vertex *vertex = (PosTexCoord0Vertex *) vb.data;

        const float minx = -_width;
        const float maxx = _width;
        const float miny = 0.0f;
        const float maxy = _height * 2.0f;

        const float texelHalfW = _texelHalf / _textureWidth;
        const float texelHalfH = _texelHalf / _textureHeight;
        const float minu = -1.0f + texelHalfW;
        const float maxu = 1.0f + texelHalfH;

        const float zz = 0.0f;

        float minv = texelHalfH;
        float maxv = 2.0f + texelHalfH;

        if (_originBottomLeft) {
            float temp = minv;
            minv = maxv;
            maxv = temp;

            minv -= 1.0f;
            maxv -= 1.0f;
        }

        vertex[0].m_x = minx;
        vertex[0].m_y = miny;
        vertex[0].m_z = zz;
        vertex[0].m_u = minu;
        vertex[0].m_v = minv;

        vertex[1].m_x = maxx;
        vertex[1].m_y = miny;
        vertex[1].m_z = zz;
        vertex[1].m_u = maxu;
        vertex[1].m_v = minv;

        vertex[2].m_x = maxx;
        vertex[2].m_y = maxy;
        vertex[2].m_z = zz;
        vertex[2].m_u = maxu;
        vertex[2].m_v = maxv;

        bgfx::setVertexBuffer(0, &vb);
    }
}


struct fullscree_pass_data {
    uint8_t input_count;
    char input_name[8][32];
    uint64_t input_resource[8];
};


#ifdef CETECH_CAN_COMPILE

static void foreach_input(yaml_node_t key,
                          yaml_node_t value,
                          void *_data) {
    fullscree_pass_data *pass_data = static_cast<fullscree_pass_data *>(_data);

    char str_buffer[128] = {};
    yaml_as_string(key,
                   str_buffer,
                   CETECH_ARRAY_LEN(
                           str_buffer) - 1);

    memcpy(&pass_data->input_name[pass_data->input_count],
           str_buffer,
           sizeof(char) *
           strlen(str_buffer) + 1);

    yaml_as_string(value,
                   str_buffer,
                   CETECH_ARRAY_LEN(
                           str_buffer) - 1);

    uint64_t resource_name = ct_hash_a0.id64_from_str(
            str_buffer);

    pass_data->input_resource[pass_data->input_count] = resource_name;

    ++pass_data->input_count;
}

static int fullscreen_pass_compiler(yaml_node_t body,
                                    struct ct_blob *data) {
    yaml_node_t input = yaml_get_node(body, "input");

    if (yaml_is_valid(input)) {
        fullscree_pass_data pass_data = {};

        yaml_node_foreach_dict(input, foreach_input, &pass_data);

        data->push(data->inst, &pass_data, sizeof(pass_data));
    }

    return 1;
}

#endif

static void fullscreen_pass(viewport_instance *viewport,
                            ct_viewport viewport_id,
                            uint8_t viewid,
                            uint8_t layerid,
                            ct_world world,
                            ct_camera camera) {

    static ct_material copy_material = ct_material_a0.resource_create(
            ct_hash_a0.id64_from_str("copy"));

    bgfx::setViewRect(viewid, 0, 0,
                      (uint16_t) viewport->size[0],  // TODO: SHITTT
                      (uint16_t) viewport->size[1]); // TODO: SHITTT

    auto fb = viewport->framebuffers[layerid];
    bgfx::setViewFrameBuffer(viewid, {fb});

    float proj[16];
    celib::mat4_ortho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, true);

    bgfx::setViewTransform(viewid, NULL, proj);

    auto &layer_entry = viewport->layers[layerid];

    screenspace_quad(viewport->size[0], viewport->size[1], 0.0f, true);

    fullscree_pass_data *pass_data = reinterpret_cast<fullscree_pass_data *>(&viewport->layers_data[viewport->layers_data_offset[layerid]]);

    for (uint8_t i = 0; i < pass_data->input_count; ++i) {
        auto input_tex = ct_viewport_a0.get_local_resource(
                viewport_id, pass_data->input_resource[i]);

        ct_material_a0.set_texture_handler(
                copy_material,
                pass_data->input_name[i],
                {input_tex});

    }

    ct_material_a0.submit(copy_material, layer_entry.name,
                          viewid);
}

void _init(struct ct_api_a0 *api) {
#ifdef CETECH_CAN_COMPILE
    ct_viewport_a0.register_pass_compiler(
            ct_hash_a0.id64_from_str("fullscreen"), fullscreen_pass_compiler);
#endif

    ct_viewport_a0.register_layer_pass(ct_hash_a0.id64_from_str("fullscreen"),
                                       fullscreen_pass);

    PosTexCoord0Vertex::init();
}

void _shutdown() {

}

CETECH_MODULE_DEF(
        fullscreen_pass,
        {
            CETECH_GET_API(api, ct_viewport_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_material_a0);
        },
        {
            _init(api);
        },
        {
            CEL_UNUSED(api);

            _shutdown();
        }
)