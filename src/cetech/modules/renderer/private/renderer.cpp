//==============================================================================
// includes
//==============================================================================
#include <cstdio>
#include <unitypes.h>

#include <celib/allocator.h>
#include <celib/array.inl>
#include <celib/map.inl>
#include <celib/handler.inl>
#include <celib/fpumath.h>

#include <cetech/core/yaml/yaml.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/config/config.h>
#include <cetech/core/macros.h>
#include <cetech/core/module/module.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/os/vio.h>

#include <cetech/engine/application/application.h>
#include <cetech/engine/machine/window.h>
#include <cetech/engine/entity/entity.h>
#include <cetech/engine/input/input.h>
#include <cetech/engine/resource/resource.h>

#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/camera/camera.h>
#include <cetech/modules/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/modules/debugui/debugui.h>

#include "bgfx/platform.h"

#include "texture/texture.h"
#include "shader/shader.h"
#include "scene/scene.h"
#include "material/material.h"
#include "mesh_renderer/mesh_renderer_private.h"

CETECH_DECL_API(ct_mesh_renderer_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_window_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_mouse_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_app_a0);

//==============================================================================
// GLobals
//==============================================================================

typedef struct {
    uint64_t name;
    uint64_t type;
    uint64_t format;
    uint64_t ration;
} render_resource_t;


typedef struct {
    uint64_t name;
    uint64_t layer;
} viewport_entry_t;


#define _G RendererGlobals
static struct G {
    celib::Map<bgfx::TextureHandle> global_resource;

    celib::Map<ct_renderer_on_pass_t> on_pass;
    celib::Array<ct_render_on_render> on_render;

    celib::Map<uint32_t> viewport_instance_map;
    celib::Array<viewport_instance> viewport_instances;
    celib::Handler<uint32_t> viewport_handler;

    ct_window *main_window;
    uint64_t type;

    uint32_t size_width;
    uint32_t size_height;

    int capture;
    int vsync;
    int need_reset;
} _G = {};

struct GConfig {
    ct_cvar screen_x;
    ct_cvar screen_y;
    ct_cvar screen_vsync;
    ct_cvar fullscreen;
    ct_cvar daemon;
    ct_cvar wid;
    ct_cvar render_config_name;
} GConfig;

//==============================================================================
// Private
//==============================================================================
struct PosTexCoord0Vertex
{
    float m_x;
    float m_y;
    float m_z;
    float m_u;
    float m_v;

    static void init()
    {
        ms_decl
                .begin()
                .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .end();
    }

    static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosTexCoord0Vertex::ms_decl;


void screenSpaceQuad(float _textureWidth, float _textureHeight, float _texelHalf, bool _originBottomLeft, float _width = 1.0f, float _height = 1.0f)
{
    if (3 == bgfx::getAvailTransientVertexBuffer(3, PosTexCoord0Vertex::ms_decl) )
    {
        bgfx::TransientVertexBuffer vb;
        bgfx::allocTransientVertexBuffer(&vb, 3, PosTexCoord0Vertex::ms_decl);
        PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

        const float minx = -_width;
        const float maxx =  _width;
        const float miny = 0.0f;
        const float maxy = _height*2.0f;

        const float texelHalfW = _texelHalf/_textureWidth;
        const float texelHalfH = _texelHalf/_textureHeight;
        const float minu = -1.0f + texelHalfW;
        const float maxu =  1.0f + texelHalfH;

        const float zz = 0.0f;

        float minv = texelHalfH;
        float maxv = 2.0f + texelHalfH;

        if (_originBottomLeft)
        {
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

static uint32_t _get_reset_flags() {
    return (_G.capture ? BGFX_RESET_CAPTURE : 0) |
           (_G.vsync ? BGFX_RESET_VSYNC : 0);
}

bgfx::TextureFormat::Enum format_id_to_enum(uint64_t id) {
    static struct {
        uint64_t id;
        bgfx::TextureFormat::Enum e;
    } _FormatIdToEnum[] = {
            {.id = ct_hash_a0.id64_from_str(
                    ""), .e = bgfx::TextureFormat::Count},
            {.id = ct_hash_a0.id64_from_str(
                    "RGBA8"), .e = bgfx::TextureFormat::RGBA8},
            {.id = ct_hash_a0.id64_from_str(
                    "D24"), .e = bgfx::TextureFormat::D24},
    };

    for (int i = 1; i < CETECH_ARRAY_LEN(_FormatIdToEnum); ++i) {
        if (_FormatIdToEnum[i].id != id) {
            continue;
        }

        return _FormatIdToEnum[i].e;
    }

    return _FormatIdToEnum[0].e;
}

bgfx::BackbufferRatio::Enum ratio_id_to_enum(uint64_t id) {
    static struct {
        uint64_t id;
        bgfx::BackbufferRatio::Enum e;
    } _RatioIdToEnum[] = {
            {.id = ct_hash_a0.id64_from_str(
                    ""), .e = bgfx::BackbufferRatio::Count},
            {.id = ct_hash_a0.id64_from_str(
                    "equal"), .e = bgfx::BackbufferRatio::Equal},
    };

    for (int i = 1; i < CETECH_ARRAY_LEN(_RatioIdToEnum); ++i) {
        if (_RatioIdToEnum[i].id != id) {
            continue;
        }

        return _RatioIdToEnum[i].e;
    }

    return _RatioIdToEnum[0].e;
}


//==============================================================================
// render global
//==============================================================================



#include "renderconfig_blob.h"


//==============================================================================
// Interface
//==============================================================================

void renderer_create() {

    if (!ct_config_a0.get_int(GConfig.daemon)) {
        int w, h;
        w = ct_config_a0.get_int(GConfig.screen_x);
        h = ct_config_a0.get_int(GConfig.screen_y);
        _G.size_width = w;
        _G.size_height = h;

        intptr_t wid = ct_config_a0.get_int(GConfig.wid);

        char title[128] = {};
        snprintf(title, CETECH_ARRAY_LEN(title), "cetech");

        if (wid == 0) {
            _G.main_window = ct_window_a0.create(
                    ct_memory_a0.main_allocator(),
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    w, h,
                    ct_config_a0.get_int(GConfig.fullscreen)
                    ? WINDOW_FULLSCREEN : WINDOW_NOFLAG
            );
        } else {
            _G.main_window = ct_window_a0.create_from(
                    ct_memory_a0.main_allocator(), (void *) wid);
        }
    }

    bgfx::PlatformData pd = {};
    pd.nwh = _G.main_window->native_window_ptr(_G.main_window->inst);
    pd.ndt = _G.main_window->native_display_ptr(_G.main_window->inst);
    bgfx::setPlatformData(pd);

    // TODO: from config
    bgfx::init(bgfx::RendererType::OpenGL, 0, 0, NULL, NULL);

    _G.main_window->size(_G.main_window->inst, &_G.size_width, &_G.size_height);

    bgfx::reset(_G.size_width, _G.size_height, _get_reset_flags());

    _G.main_window->update(_G.main_window);

    _G.need_reset = 1;
}

void renderer_register_layer_pass(uint64_t type,
                                  ct_renderer_on_pass_t on_pass) {
    celib::map::set(_G.on_pass, type, on_pass);
}


struct ct_texture renderer_get_global_resource(uint64_t name) {
    auto idx = celib::map::get(_G.global_resource, name,
                               BGFX_INVALID_HANDLE).idx;
    return {.idx = idx};
}

void renderer_set_debug(int debug) {
    if (debug) {
        bgfx::setDebug(BGFX_DEBUG_STATS);
    } else {
        bgfx::setDebug(BGFX_DEBUG_NONE);
    }
}

static int viewid_counter = 0;

void renderer_render_world(ct_world world,
                           ct_camera camera,
                           ct_viewport viewport) {

    auto idx = celib::map::get(_G.viewport_instance_map, viewport.idx,
                               UINT32_MAX);
    auto &vi = _G.viewport_instances[idx];

    for (uint8_t i = 0; i < vi.layer_count; ++i) {
        auto &entry = vi.layers[i];

        auto *on_pass = celib::map::get<ct_renderer_on_pass_t>(_G.on_pass,
                                                               entry.type,
                                                               NULL);
        if (!on_pass) {
            continue;
        }

        on_pass(&vi, viewid_counter++, i, world, camera);
    }
}

void on_render() {
    if (_G.need_reset) {
        _G.need_reset = 0;

        // bgfx::reset(_G.size_width, _G.size_height, _get_reset_flags());
    }

    for (uint32_t i = 0; i < celib::array::size(_G.on_render); ++i) {
        _G.on_render[i]();
    }

    bgfx::frame();
    _G.main_window->update(_G.main_window);
    viewid_counter = 0;
}

void renderer_get_size(uint32_t *width,
                       uint32_t *height) {
    *width = _G.size_width;
    *height = _G.size_height;
}


uint16_t _render_viewport_get_local_resource(viewport_instance& instance,
                                            uint64_t name) {


    for (int i = 0; i < instance.resource_count; ++i) {
        if (instance.local_resource_name[i] != name) {
            continue;
        }

        return instance.local_resource[i];
    }

    return UINT16_MAX;
}

uint16_t render_viewport_get_local_resource(ct_viewport viewport,
                                            uint64_t name) {

    auto idx = celib::map::get(_G.viewport_instance_map, viewport.idx,
                               UINT32_MAX);
    auto &vi = _G.viewport_instances[idx];

    return _render_viewport_get_local_resource(vi, name);
}

ct_viewport renderer_create_viewport(uint64_t name,
                                     float width,
                                     float height) {
    auto id = celib::handler::create(_G.viewport_handler);

    auto *resource = ct_resource_a0.get(_G.type, name);
    auto *blob = renderconfig_blob::get(resource);

    auto idx = celib::array::size(_G.viewport_instances);

    viewport_instance vi = {};

    if (!width || !height) {
        uint32_t w, h;
        renderer_get_size(&w, &h);
        width = float(w);
        height = float(h);
    }

    vi.size[0] = width;
    vi.size[1] = height;

    for (uint32_t i = 0; i < blob->viewport_count; ++i) {
        auto &vp = renderconfig_blob::viewport(blob)[i];
        if (vp.name != name) {
            continue;
        }


        for (uint32_t j = 0; j < blob->layer_count; ++j) {
            auto layer_name = renderconfig_blob::layers_name(blob)[j];
            if (layer_name != vp.layer) {
                continue;
            }

            auto localresource_count = renderconfig_blob::layers_localresource_count(
                    blob)[j];
            auto localresource_offset = renderconfig_blob::layers_localresource_offset(
                    blob)[j];
            auto *localresource =
                    renderconfig_blob::local_resource(blob) + localresource_offset;

            vi.resource_count = localresource_count;

            for (int k = 0; k < localresource_count; ++k) {
                auto &lr = localresource[k];

                auto format = format_id_to_enum(lr.format);
                auto ration = ratio_id_to_enum(lr.ration);

                const uint32_t samplerFlags = 0
                                              | BGFX_TEXTURE_RT
                                              | BGFX_TEXTURE_MIN_POINT
                                              | BGFX_TEXTURE_MAG_POINT
                                              | BGFX_TEXTURE_MIP_POINT
                                              | BGFX_TEXTURE_U_CLAMP
                                              | BGFX_TEXTURE_V_CLAMP;

                auto h = bgfx::createTexture2D(ration, false, 1, format,
                                               samplerFlags);

                vi.local_resource[k] = h.idx;
                vi.local_resource_name[k] = lr.name;
            }

            auto entry_count = renderconfig_blob::layers_entry_count(blob)[j];
            auto entry_offset = renderconfig_blob::layers_entry_offset(blob)[j];
            auto *entry = renderconfig_blob::layers_entry(blob) + entry_offset;

            vi.layer_count = entry_count;
            vi.layers = entry;

            for (int k = 0; k < entry_count; ++k) {
                auto &e = entry[k];

                bgfx::TextureHandle th[e.output_count];
                for (int l = 0; l < e.output_count; ++l) {
                    // TODO if not found in local then find in global
                    th[l] = {_render_viewport_get_local_resource(vi, e.output[l])};
                }

                auto fb = bgfx::createFrameBuffer(e.output_count, th, false);
                vi.framebuffers[vi.fb_count] = fb.idx;
                ++vi.fb_count;
            }

        }
    }

    celib::map::set(_G.viewport_instance_map, id, idx);
    celib::array::push_back(_G.viewport_instances, vi);

    return {.idx = id};
}


//==============================================================================
// render_config resource
//==============================================================================
namespace renderconfig_resource {


    void *loader(ct_vio *input,
                 cel_alloc *allocator) {
        const int64_t size = input->size(input->inst);
        char *data = CEL_ALLOCATE(allocator, char, size);
        input->read(input->inst, data, 1, size);
        return data;
    }

    void unloader(void *new_data,
                  cel_alloc *allocator) {
        CEL_FREE(allocator, new_data);
    }

    void online(uint64_t name,
                void *data) {
        CEL_UNUSED(name);

        auto *blob = renderconfig_blob::get(data);

        for (uint32_t i = 0; i < blob->global_resource_count; ++i) {
            auto &gr = renderconfig_blob::global_resource(blob)[i];

            auto format = format_id_to_enum(gr.format);
            auto ration = ratio_id_to_enum(gr.ration);

            const uint32_t samplerFlags = 0
                                          | BGFX_TEXTURE_RT
                                          | BGFX_TEXTURE_MIN_POINT
                                          | BGFX_TEXTURE_MAG_POINT
                                          | BGFX_TEXTURE_MIP_POINT
                                          | BGFX_TEXTURE_U_CLAMP
                                          | BGFX_TEXTURE_V_CLAMP;

            auto h = bgfx::createTexture2D(ration, false, 1, format,
                                           samplerFlags);

            celib::map::set(_G.global_resource, gr.name, h);
        }

    }

    void offline(uint64_t name,
                 void *data) {
        CEL_UNUSED(name, data);
    }

    void *reloader(uint64_t name,
                   void *old_data,
                   void *new_data,
                   cel_alloc *allocator) {
        offline(name, old_data);
        online(name, new_data);

        CEL_FREE(allocator, old_data);
        return new_data;
    }

    static const ct_resource_callbacks_t callback = {
            .loader = loader,
            .unloader = unloader,
            .online = online,
            .offline = offline,
            .reloader = reloader
    };
}

//// COMPIELr
namespace renderconfig_compiler {
    namespace {
        struct compiler_output {
            celib::Array<render_resource_t> global_resource;
            celib::Array<render_resource_t> local_resource;
            celib::Array<uint64_t> layer_names;
            celib::Array<uint32_t> layers_entry_count;
            celib::Array<uint32_t> layers_localresource_count;
            celib::Array<uint32_t> layers_localresource_offset;
            celib::Array<uint32_t> layers_entry_offset;
            celib::Array<layer_entry_t> layers_entry;
            celib::Array<viewport_entry_t> viewport;
        };


        int compiler(const char *filename,
                     ct_vio *source_vio,
                     ct_vio *build_vio,
                     ct_compilator_api *compilator_api) {

            char *source_data =
                    CEL_ALLOCATE(ct_memory_a0.main_allocator(), char,
                                 source_vio->size(source_vio->inst) + 1);
            memset(source_data, 0, source_vio->size(source_vio->inst) + 1);

            source_vio->read(source_vio->inst, source_data, sizeof(char),
                             source_vio->size(source_vio->inst));

            yaml_document_t h;
            yaml_node_t root = yaml_load_str(source_data, &h);

            struct compiler_output output = {};
            output.global_resource.init(ct_memory_a0.main_allocator());
            output.layer_names.init(ct_memory_a0.main_allocator());
            output.layers_entry_count.init(ct_memory_a0.main_allocator());
            output.layers_entry_offset.init(ct_memory_a0.main_allocator());
            output.layers_entry.init(ct_memory_a0.main_allocator());
            output.viewport.init(ct_memory_a0.main_allocator());
            output.local_resource.init(ct_memory_a0.main_allocator());
            output.layers_localresource_count.init(
                    ct_memory_a0.main_allocator());
            output.layers_localresource_offset.init(
                    ct_memory_a0.main_allocator());


            //==================================================================
            // Global resource
            //==================================================================
            yaml_node_t global_resource = yaml_get_node(root,
                                                        "global_resource");
            if (yaml_is_valid(global_resource)) {
                yaml_node_foreach_seq(
                        global_resource,
                        [](uint32_t idx,
                           yaml_node_t value,
                           void *_data) {
                            char str_buffer[128] = {};
                            render_resource_t gs = {};

                            compiler_output &output = *((compiler_output *) _data);

                            ////////////////////////////////////////////////////
                            yaml_node_t name = yaml_get_node(value, "name");
                            yaml_as_string(name, str_buffer,
                                           CETECH_ARRAY_LEN(str_buffer) - 1);

                            gs.name = ct_hash_a0.id64_from_str(str_buffer);


                            /////////////////////////////////////////////////////
                            yaml_node_t type = yaml_get_node(value, "type");
                            yaml_as_string(type, str_buffer,
                                           CETECH_ARRAY_LEN(str_buffer) - 1);

                            gs.type = ct_hash_a0.id64_from_str(str_buffer);


                            /////////////////////////////////////////////////////
                            yaml_node_t format = yaml_get_node(value, "format");
                            yaml_as_string(format, str_buffer,
                                           CETECH_ARRAY_LEN(str_buffer) - 1);

                            gs.format = ct_hash_a0.id64_from_str(str_buffer);

                            /////////////////////////////////////////////////////
                            yaml_node_t ration = yaml_get_node(value, "ration");
                            yaml_as_string(ration, str_buffer,
                                           CETECH_ARRAY_LEN(str_buffer) - 1);

                            gs.ration = ct_hash_a0.id64_from_str(str_buffer);


                            celib::array::push_back(output.global_resource, gs);
                        }, &output);
            }

            //==================================================================
            // layers
            //==================================================================
            yaml_node_t layers = yaml_get_node(root, "layers");
            if (yaml_is_valid(layers)) {

                yaml_node_foreach_dict(
                        layers,
                        [](yaml_node_t key,
                           yaml_node_t value,
                           void *_data) {
                            char str_buffer[128] = {};

                            compiler_output &output = *((compiler_output *) _data);

                            yaml_as_string(key, str_buffer,
                                           CETECH_ARRAY_LEN(str_buffer) - 1);

                            auto name_id = ct_hash_a0.id64_from_str(str_buffer);
                            auto layer_offset = celib::array::size(
                                    output.layers_entry);

                            celib::array::push_back(output.layer_names,
                                                    name_id);
                            celib::array::push_back(output.layers_entry_offset,
                                                    layer_offset);

                            yaml_node_foreach_seq(
                                    value,
                                    [](uint32_t idx,
                                       yaml_node_t value,
                                       void *_data) {
                                        char str_buffer[128] = {};
                                        layer_entry_t le = {};

                                        auto &output = *((compiler_output *) _data);

                                        ////////////////////////////////////////////////////
                                        yaml_node_t name = yaml_get_node(value,
                                                                         "name");
                                        yaml_as_string(name, str_buffer,
                                                       CETECH_ARRAY_LEN(
                                                               str_buffer) - 1);

                                        le.name = ct_hash_a0.id64_from_str(
                                                str_buffer);


                                        /////////////////////////////////////////////////////
                                        yaml_node_t type = yaml_get_node(value,
                                                                         "type");

                                        yaml_as_string(type, str_buffer,
                                                       CETECH_ARRAY_LEN(
                                                               str_buffer) - 1);

                                        le.type = ct_hash_a0.id64_from_str(
                                                str_buffer);

                                        /////////////////////////////////////////////////////
                                        yaml_node_t input = yaml_get_node(value,
                                                                          "input");
                                        if (yaml_is_valid(input)) {
                                            yaml_node_foreach_seq(
                                                    input,
                                                    [](uint32_t idx,
                                                       yaml_node_t value,
                                                       void *_data) {
                                                        char str_buffer[128] = {};
                                                        auto &le = *((layer_entry_t *) _data);

                                                        yaml_as_string(value,
                                                                       str_buffer,
                                                                       CETECH_ARRAY_LEN(
                                                                               str_buffer) -
                                                                       1);

                                                        le.input[idx] = ct_hash_a0.id64_from_str(
                                                                str_buffer);
                                                        ++le.input_count;

                                                    }, &le);
                                        }

                                        /////////////////////////////////////////////////////
                                        yaml_node_t output_t = yaml_get_node(
                                                value,
                                                "output");
                                        if (yaml_is_valid(output_t)) {
                                            yaml_node_foreach_seq(
                                                    output_t,
                                                    [](uint32_t idx,
                                                       yaml_node_t value,
                                                       void *_data) {
                                                        char str_buffer[128] = {};
                                                        auto &le = *((layer_entry_t *) _data);

                                                        yaml_as_string(value,
                                                                       str_buffer,
                                                                       CETECH_ARRAY_LEN(
                                                                               str_buffer) -
                                                                       1);

                                                        le.output[idx] = ct_hash_a0.id64_from_str(
                                                                str_buffer);
                                                        ++le.output_count;

                                                    }, &le);
                                        }


                                        celib::array::push_back(
                                                output.layers_entry, le);

                                    }, &output);

                            celib::array::push_back(output.layers_entry_count,
                                                    celib::array::size(
                                                            output.layers_entry) -
                                                    layer_offset);

                        }, &output);
            }

            //==================================================================
            // Viewport
            //==================================================================
            yaml_node_t viewport = yaml_get_node(root, "viewport");
            if (yaml_is_valid(viewport)) {

                yaml_node_foreach_dict(
                        viewport,
                        [](yaml_node_t key,
                           yaml_node_t value,
                           void *_data) {
                            char str_buffer[128] = {};

                            compiler_output &output = *((compiler_output *) _data);

                            yaml_as_string(key, str_buffer,
                                           CETECH_ARRAY_LEN(str_buffer) - 1);

                            auto name_id = ct_hash_a0.id64_from_str(str_buffer);

                            yaml_as_string(key, str_buffer,
                                           CETECH_ARRAY_LEN(str_buffer) - 1);

                            yaml_node_t layers = yaml_get_node(value, "layers");
                            yaml_as_string(layers, str_buffer,
                                           CETECH_ARRAY_LEN(str_buffer) - 1);
                            auto layers_id = ct_hash_a0.id64_from_str(
                                    str_buffer);


                            viewport_entry_t ve = {};

                            ve.name = name_id;
                            ve.layer = layers_id;

                            celib::array::push_back(output.viewport, ve);

                            //////
                            auto localresource_offset = celib::array::size(
                                    output.local_resource);

                            celib::array::push_back(
                                    output.layers_localresource_offset,
                                    localresource_offset);

                            yaml_node_t local_resource = yaml_get_node(value,
                                                                       "local_resource");
                            if (yaml_is_valid(local_resource)) {
                                yaml_node_foreach_seq(
                                        local_resource,
                                        [](uint32_t idx,
                                           yaml_node_t value,
                                           void *_data) {
                                            char str_buffer[128] = {};
                                            render_resource_t gs = {};

                                            compiler_output &output = *((compiler_output *) _data);

                                            ////////////////////////////////////////////////////
                                            yaml_node_t name = yaml_get_node(
                                                    value, "name");
                                            yaml_as_string(name, str_buffer,
                                                           CETECH_ARRAY_LEN(
                                                                   str_buffer) -
                                                           1);

                                            gs.name = ct_hash_a0.id64_from_str(
                                                    str_buffer);


                                            /////////////////////////////////////////////////////
                                            yaml_node_t type = yaml_get_node(
                                                    value, "type");
                                            yaml_as_string(type, str_buffer,
                                                           CETECH_ARRAY_LEN(
                                                                   str_buffer) -
                                                           1);

                                            gs.type = ct_hash_a0.id64_from_str(
                                                    str_buffer);


                                            /////////////////////////////////////////////////////
                                            yaml_node_t format = yaml_get_node(
                                                    value, "format");
                                            yaml_as_string(format, str_buffer,
                                                           CETECH_ARRAY_LEN(
                                                                   str_buffer) -
                                                           1);

                                            gs.format = ct_hash_a0.id64_from_str(
                                                    str_buffer);

                                            /////////////////////////////////////////////////////
                                            yaml_node_t ration = yaml_get_node(
                                                    value, "ration");
                                            yaml_as_string(ration, str_buffer,
                                                           CETECH_ARRAY_LEN(
                                                                   str_buffer) -
                                                           1);

                                            gs.ration = ct_hash_a0.id64_from_str(
                                                    str_buffer);


                                            celib::array::push_back(
                                                    output.local_resource, gs);
                                        }, &output);

                                celib::array::push_back(
                                        output.layers_localresource_count,
                                        celib::array::size(
                                                output.local_resource) -
                                        localresource_offset);
                            }

                        }, &output);
            }

            renderconfig_blob::blob_t resource = {
                    .global_resource_count = celib::array::size(
                            output.global_resource),

                    .layer_count = celib::array::size(
                            output.layer_names),

                    .layer_entry_count = celib::array::size(
                            output.layers_entry),

                    .local_resource_count = celib::array::size(
                            output.local_resource),

                    .viewport_count = celib::array::size(output.viewport),
            };


            build_vio->write(build_vio->inst, &resource, sizeof(resource), 1);

            build_vio->write(build_vio->inst,
                             celib::array::begin(output.global_resource),
                             sizeof(render_resource_t),
                             celib::array::size(output.global_resource));

            build_vio->write(build_vio->inst,
                             celib::array::begin(output.local_resource),
                             sizeof(render_resource_t),
                             celib::array::size(output.local_resource));

            build_vio->write(build_vio->inst,
                             celib::array::begin(output.layer_names),
                             sizeof(uint64_t),
                             celib::array::size(output.layer_names));

            build_vio->write(build_vio->inst,
                             celib::array::begin(output.layers_entry_count),
                             sizeof(uint32_t),
                             celib::array::size(output.layers_entry_count));

            build_vio->write(build_vio->inst,
                             celib::array::begin(output.layers_entry_offset),
                             sizeof(uint32_t),
                             celib::array::size(output.layers_entry_offset));


            build_vio->write(build_vio->inst,
                             celib::array::begin(
                                     output.layers_localresource_count),
                             sizeof(uint32_t),
                             celib::array::size(
                                     output.layers_localresource_count));

            build_vio->write(build_vio->inst,
                             celib::array::begin(
                                     output.layers_localresource_offset),
                             sizeof(uint32_t),
                             celib::array::size(
                                     output.layers_localresource_offset));


            build_vio->write(build_vio->inst,
                             celib::array::begin(output.layers_entry),
                             sizeof(layer_entry_t),
                             celib::array::size(output.layers_entry));

            build_vio->write(build_vio->inst,
                             celib::array::begin(output.viewport),
                             sizeof(viewport_entry_t),
                             celib::array::size(output.viewport));

            output.global_resource.destroy();
            output.layer_names.destroy();
            output.layers_entry_count.destroy();
            output.layers_entry_offset.destroy();
            output.layers_entry.destroy();
            output.viewport.destroy();
            output.local_resource.destroy();
            output.layers_localresource_count.destroy();
            output.layers_localresource_offset.destroy();

            CEL_FREE(ct_memory_a0.main_allocator(), source_data);
            return 1;
        }

        int init(struct ct_api_a0 *api) {
            ct_resource_a0.compiler_register(
                    ct_hash_a0.id64_from_str("render_config"),
                    compiler);

            return 1;
        }
    }
}
/////
#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                                   \
        celib::array::push_back(_G.name, name);                                \
    }                                                                          \
    static void unregister_## name ## _(type name) {                                  \
        const auto size = celib::array::size(_G.name);                         \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            celib::array::pop_back(_G.name);                                   \
            break;                                                             \
        }                                                                      \
    }

_DEF_ON_CLB_FCE(ct_render_on_render, on_render)

#undef _DEF_ON_CLB_FCE


namespace renderer_module {
    static ct_renderer_a0 rendderer_api = {
            .create = renderer_create,
            .set_debug = renderer_set_debug,
            .get_size = renderer_get_size,
            .render_world = renderer_render_world,
            .register_layer_pass = renderer_register_layer_pass,
            .get_global_resource = renderer_get_global_resource,
            .register_on_render =register_on_render_,
            .unregister_on_render =unregister_on_render_,
            .create_viewport = renderer_create_viewport,
            .viewport_get_local_resource = render_viewport_get_local_resource,
    };

    static struct ct_material_a0 material_api = {
            .resource_create = material::create,
            .set_texture = material::set_texture,
            .set_texture2 = material::set_texture2,
            .set_mat44f = material::set_mat44f,
            .submit = material::submit
    };

    void _init_api(struct ct_api_a0 *api) {
        api->register_api("ct_renderer_a0", &rendderer_api);
        api->register_api("ct_material_a0", &material_api);
    }


    void _init(struct ct_api_a0 *api) {
        _init_api(api);

        ct_api_a0 = *api;

        _G = (struct G) {};


        GConfig = {
                .screen_x = ct_config_a0.new_int(
                        "screen.x", "Screen width", 1024),

                .screen_y = ct_config_a0.new_int(
                        "screen.y", "Screen height", 768),

                .screen_vsync = ct_config_a0.new_int(
                        "screen.vsync", "Screen vsync", 1),

                .fullscreen = ct_config_a0.new_int(
                        "screen.fullscreen", "Fullscreen", 0),

                .daemon = ct_config_a0.new_int("daemon", "Daemon mode", 0),
                .wid = ct_config_a0.new_int("wid", "Wid", 0),

                .render_config_name = ct_config_a0.new_str("renderer.config",
                                                           "Render condfig",
                                                           "default")
        };

        if (!ct_config_a0.get_int(GConfig.daemon)) {
            texture::texture_init(api);
            shader::shader_init(api);
            material::init(api);
            scene::init(api);
            mesh::init(api);

            _G.vsync = ct_config_a0.get_int(GConfig.screen_vsync) > 0;

            CETECH_GET_API(api, ct_mesh_renderer_a0);
        }

        CETECH_GET_API(api, ct_window_a0);

        renderer_create();

        _G.global_resource.init(ct_memory_a0.main_allocator());
        _G.on_pass.init(ct_memory_a0.main_allocator());
        _G.on_render.init(ct_memory_a0.main_allocator());

        _G.viewport_instance_map.init(ct_memory_a0.main_allocator());
        _G.viewport_instances.init(ct_memory_a0.main_allocator());
        _G.viewport_handler.init(ct_memory_a0.main_allocator());

        _G.type = ct_hash_a0.id64_from_str("render_config");
        ct_resource_a0.register_type(_G.type,
                                     renderconfig_resource::callback);

#ifdef CETECH_CAN_COMPILE
        renderconfig_compiler::init(api);
#endif

        renderer_register_layer_pass(
                ct_hash_a0.id64_from_str("geometry"),
                [](viewport_instance *viewport,
                   uint8_t viewid,
                   uint8_t layerid,
                   ct_world world,
                   ct_camera camera) {

                    ct_camera_a0 *camera_api = (ct_camera_a0 *) ct_api_a0.first(
                            "ct_camera_a0").api; // TODO: SHIT !!!!

                    bgfx::setViewClear(viewid,
                                       BGFX_CLEAR_COLOR |
                                       BGFX_CLEAR_DEPTH,
                                       0x66CCFFff,
                                       1.0f, 0);

                    bgfx::setViewRect(viewid, 0, 0,
                                      (uint16_t) viewport->size[0],  // TODO: SHITTT
                                      (uint16_t) viewport->size[1]); // TODO: SHITTT

                    float view_matrix[16];
                    float proj_matrix[16];

                    camera_api->get_project_view(camera,
                                                 proj_matrix,
                                                 view_matrix,
                                                 _G.size_width,
                                                 _G.size_height);

                    auto fb = viewport->framebuffers[layerid];
                    bgfx::setViewFrameBuffer(viewid, {fb});

                    bgfx::setViewTransform(viewid, view_matrix,
                                           proj_matrix);

                    // TODO: CULLING
                    ct_mesh_renderer_a0.render_all(world, viewid, viewport->layers[layerid].name);
                });


        renderer_register_layer_pass(
                ct_hash_a0.id64_from_str("copy"),
                [](viewport_instance *viewport,
                   uint8_t viewid,
                   uint8_t layerid,
                   ct_world world,
                   ct_camera camera) {
                    static ct_material copy_material = material_api.resource_create(ct_hash_a0.id64_from_str("copy"));


                    bgfx::setViewRect(viewid, 0, 0,
                                      (uint16_t) viewport->size[0],  // TODO: SHITTT
                                      (uint16_t) viewport->size[1]); // TODO: SHITTT

                    auto fb = viewport->framebuffers[layerid];
                    bgfx::setViewFrameBuffer(viewid, {fb});

                    float proj[16];
                    celib::mat4_ortho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, true);

                    bgfx::setViewTransform(viewid, NULL, proj);

                    auto& layer_entry = viewport->layers[layerid];

                    screenSpaceQuad( viewport->size[0], viewport->size[1], 0.0f, true);

                    auto input_tex = _render_viewport_get_local_resource(*viewport, layer_entry.input[0]);

                    material_api.set_texture2(copy_material, "s_input_texture", {input_tex});
                    material_api.submit(copy_material, layer_entry.name, viewid);
                });


        ct_app_a0.register_on_render(on_render);

        PosTexCoord0Vertex::init();
    }

    void _shutdown() {
        ct_cvar daemon = ct_config_a0.find("daemon");
        if (!ct_config_a0.get_int(daemon)) {
            ct_app_a0.unregister_on_render(on_render);

            texture::texture_shutdown();
            shader::shader_shutdown();
            material::shutdown();
            scene::shutdown();
            mesh::shutdown();

            _G.global_resource.destroy();
            _G.on_pass.destroy();
            _G.on_render.destroy();

            _G.viewport_instance_map.destroy();
            _G.viewport_instances.destroy();
            _G.viewport_handler.destroy();

            bgfx::shutdown();
        }

        _G = (struct G) {};
    }

}

CETECH_MODULE_DEF(
        renderer,
        {
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_mouse_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_app_a0);
        },
        {
            renderer_module::_init(api);
        },
        {
            CEL_UNUSED(api);

            renderer_module::_shutdown();

        }
)