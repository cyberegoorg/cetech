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
#include <cetech/memory/memory.h>
#include <cetech/hashlib/hashlib.h>
#include <cetech/os/vio.h>

#include <cetech/application/application.h>
#include <cetech/machine/window.h>
#include <cetech/entity/entity.h>
#include <cetech/input/input.h>
#include <cetech/resource/resource.h>

#include <cetech/renderer/renderer.h>
#include <cetech/renderer/texture.h>
#include <cetech/camera/camera.h>
#include <cetech/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/debugui/debugui.h>
#include <cetech/machine/machine.h>
#include <cetech/renderer/mesh_renderer.h>
#include <cetech/blob/blob.h>


#include "bgfx/platform.h"

#include <cetech/renderer/viewport.h>
#include "cetech/renderer/scene.h"
#include "cetech/renderer/material.h"

CETECH_DECL_API(ct_mesh_renderer_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_window_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_mouse_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_machine_a0);
CETECH_DECL_API(ct_material_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_blob_a0);

using namespace celib;

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
    Map<bgfx::TextureHandle> global_resource;
    Map<ct_viewport_on_pass_t> on_pass;
    Map<uint32_t> viewport_instance_map;
    Array<viewport_instance> viewport_instances;
    Handler<uint32_t> viewport_handler;

    Map<ct_viewport_pass_compiler> compiler_map;

    uint64_t type;

    uint32_t size_width;
    uint32_t size_height;
    int need_reset;
} _G = {};

static struct GConfig {
    ct_cvar render_config_name;
} GConfig;

//==============================================================================
// Private
//==============================================================================
#include "viewport_enums.h"

//==============================================================================
// render global
//==============================================================================

#include "renderconfig_blob.h"


//==============================================================================
// Interface
//==============================================================================

void renderer_register_layer_pass(uint64_t type,
                                  ct_viewport_on_pass_t on_pass) {
    map::set(_G.on_pass, type, on_pass);
}


struct ct_texture renderer_get_global_resource(uint64_t name) {
    auto idx = map::get(_G.global_resource, name,
                        BGFX_INVALID_HANDLE).idx;
    return {.idx = idx};
}


static int viewid_counter = 0;

void renderer_render_world(ct_world world,
                           ct_camera camera,
                           ct_viewport viewport) {

    auto idx = map::get(_G.viewport_instance_map, viewport.idx,
                        UINT32_MAX);
    auto &vi = _G.viewport_instances[idx];

    for (uint8_t i = 0; i < vi.layer_count; ++i) {
        auto &entry = vi.layers[i];

        auto *on_pass = map::get<ct_viewport_on_pass_t>(_G.on_pass,
                                                        entry.type,
                                                        NULL);
        if (!on_pass) {
            continue;
        }

        on_pass(&vi, viewport, viewid_counter++, i, world, camera);
    }
}


uint16_t get_local_resource(viewport_instance &instance,
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

    auto idx = map::get(_G.viewport_instance_map, viewport.idx,
                        UINT32_MAX);
    auto &vi = _G.viewport_instances[idx];

    return get_local_resource(vi, name);
}

void _init_viewport(viewport_instance &vi,
                    uint64_t name,
                    float width,
                    float height) {

    auto *resource = ct_resource_a0.get(_G.type, name);
    auto *blob = renderconfig_blob::get(resource);

    if (!width || !height) {
        uint32_t w, h;
        ct_renderer_a0.get_size(&w, &h);
        width = float(w);
        height = float(h);
    }

    vi.size[0] = width;
    vi.size[1] = height;
    vi.viewport = name;

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
                    renderconfig_blob::local_resource(blob) +
                    localresource_offset;

            vi.resource_count = localresource_count;

            for (int k = 0; k < localresource_count; ++k) {
                auto &lr = localresource[k];

                auto format = format_id_to_enum(lr.format);
                auto ration_coef = ratio_id_to_coef(lr.ration);

                const uint32_t samplerFlags = 0
                                              | BGFX_TEXTURE_RT
                                              | BGFX_TEXTURE_MIN_POINT
                                              | BGFX_TEXTURE_MAG_POINT
                                              | BGFX_TEXTURE_MIP_POINT
                                              | BGFX_TEXTURE_U_CLAMP
                                              | BGFX_TEXTURE_V_CLAMP;

                if (0 != vi.local_resource[k]) {
                    bgfx::destroy((bgfx::TextureHandle) {vi.local_resource[k]});
                }

                auto h = bgfx::createTexture2D(
                        static_cast<uint16_t>(width * ration_coef),
                        static_cast<uint16_t>(height * ration_coef),
                        false, 1, format, samplerFlags);

                vi.local_resource[k] = h.idx;
                vi.local_resource_name[k] = lr.name;
            }

            auto entry_count = renderconfig_blob::layers_entry_count(blob)[j];
            auto entry_offset = renderconfig_blob::layers_entry_offset(blob)[j];
            auto *entry = renderconfig_blob::layers_entry(blob) + entry_offset;
            auto *data = renderconfig_blob::data(blob);
            auto *data_offset = renderconfig_blob::entry_data_offset(blob);

            vi.layer_count = entry_count;
            vi.layers = entry;

            vi.layers_data = data;
            vi.layers_data_offset = data_offset;

            vi.fb_count = entry_count;
            for (int k = 0; k < entry_count; ++k) {
                auto &e = entry[k];

                bgfx::TextureHandle th[e.output_count];
                for (int l = 0; l < e.output_count; ++l) {
                    // TODO if not found in local then find in global
                    th[l] = {get_local_resource(vi,
                                                e.output[l])};
                }

                if (0 != vi.framebuffers[k]) {
                    bgfx::destroy((bgfx::FrameBufferHandle) {
                            vi.framebuffers[k]});
                }

                auto fb = bgfx::createFrameBuffer(e.output_count, th, false);
                vi.framebuffers[k] = fb.idx;
            }
        }
    }
}

void resize_viewport(ct_viewport viewport,
                     float width,
                     float height) {

    auto idx = map::get(_G.viewport_instance_map, viewport.idx,
                        UINT32_MAX);
    auto &vi = _G.viewport_instances[idx];

    if ((width != vi.size[0]) || (height != vi.size[1])) {
        _init_viewport(vi, vi.viewport, width, height);
    }
}

void recreate_all_viewport() {
    for (uint32_t i = 0; i < array::size(_G.viewport_instances); ++i) {
        auto &vi = _G.viewport_instances[i];
        _init_viewport(vi, vi.viewport, vi.size[0], vi.size[1]);
    }
}

ct_viewport renderer_create_viewport(uint64_t name,
                                     float width,
                                     float height) {
    viewport_instance vi = {};
    _init_viewport(vi, name, width, height);

    auto idx = array::size(_G.viewport_instances);
    auto id = handler::create(_G.viewport_handler);
    map::set(_G.viewport_instance_map, id, idx);
    array::push_back(_G.viewport_instances, vi);

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

            map::set(_G.global_resource, gr.name, h);
        }

    }

    void offline(uint64_t name,
                 void *data) {
        CEL_UNUSED(name, data);
        //auto *blob = renderconfig_blob::get(data);


    }

    void *reloader(uint64_t name,
                   void *old_data,
                   void *new_data,
                   cel_alloc *allocator) {
        offline(name, old_data);
        online(name, new_data);

        recreate_all_viewport();

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
struct compiler_output {
    Array<uint64_t> layer_names;
    Array<uint32_t> layers_entry_count;
    Array<uint32_t> layers_entry_offset;
    Array<uint32_t> layers_localresource_count;
    Array<uint32_t> layers_localresource_offset;
    Array<uint32_t> entry_data_offset;
    Array<render_resource_t> global_resource;
    Array<render_resource_t> local_resource;
    Array<layer_entry_t> layers_entry;
    Array<viewport_entry_t> viewport;
    ct_blob *blob;
};

void compile_global_resource(uint32_t idx,
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


    array::push_back(output.global_resource, gs);
}


void compile_layer_entry(uint32_t idx,
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

    auto compiler = map::get<ct_viewport_pass_compiler>(_G.compiler_map,
                                                        le.type, NULL);

    auto blob = output.blob;

    array::push_back(output.entry_data_offset,
                     static_cast<const uint32_t &>(blob->size(blob->inst)));

    if (compiler) {
        compiler(value, output.blob);
    }

    array::push_back(output.layers_entry, le);

}

void compile_layers(yaml_node_t key,
                    yaml_node_t value,
                    void *_data) {
    char str_buffer[128] = {};

    compiler_output &output = *((compiler_output *) _data);

    yaml_as_string(key, str_buffer,
                   CETECH_ARRAY_LEN(str_buffer) - 1);

    auto name_id = ct_hash_a0.id64_from_str(str_buffer);
    auto layer_offset = array::size(
            output.layers_entry);

    array::push_back(output.layer_names, name_id);
    array::push_back(output.layers_entry_offset, layer_offset);

    yaml_node_foreach_seq(value, compile_layer_entry, &output);

    const uint32_t entry_count = array::size(output.layers_entry);

    array::push_back(output.layers_entry_count,
                     entry_count - layer_offset);
}

namespace renderconfig_compiler {


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
        output.entry_data_offset.init(ct_memory_a0.main_allocator());

        output.blob = ct_blob_a0.create(ct_memory_a0.main_allocator());

        //==================================================================
        // Global resource
        //==================================================================
        yaml_node_t global_resource = yaml_get_node(root,
                                                    "global_resource");
        if (yaml_is_valid(global_resource)) {
            yaml_node_foreach_seq(
                    global_resource,
                    compile_global_resource, &output);
        }

        //==================================================================
        // layers
        //==================================================================
        yaml_node_t layers = yaml_get_node(root, "layers");
        if (yaml_is_valid(layers)) {

            yaml_node_foreach_dict(
                    layers,
                    compile_layers, &output);
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

                        array::push_back(output.viewport, ve);

                        //////
                        auto localresource_offset = array::size(
                                output.local_resource);

                        array::push_back(
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


                                        array::push_back(
                                                output.local_resource, gs);
                                    }, &output);

                            array::push_back(
                                    output.layers_localresource_count,
                                    array::size(
                                            output.local_resource) -
                                    localresource_offset);
                        }

                    }, &output);
        }

        renderconfig_blob::blob_t resource = {
                .global_resource_count = array::size(
                        output.global_resource),

                .layer_count = array::size(output.layer_names),

                .layer_entry_count = array::size(output.layers_entry),

                .local_resource_count = array::size(
                        output.local_resource),

                .viewport_count = array::size(output.viewport),
        };


        build_vio->write(build_vio->inst, &resource, sizeof(resource), 1);

        build_vio->write(build_vio->inst,
                         array::begin(output.global_resource),
                         sizeof(render_resource_t),
                         array::size(output.global_resource));

        build_vio->write(build_vio->inst,
                         array::begin(output.local_resource),
                         sizeof(render_resource_t),
                         array::size(output.local_resource));

        build_vio->write(build_vio->inst,
                         array::begin(output.layer_names),
                         sizeof(uint64_t),
                         array::size(output.layer_names));

        build_vio->write(build_vio->inst,
                         array::begin(output.layers_entry_count),
                         sizeof(uint32_t),
                         array::size(output.layers_entry_count));

        build_vio->write(build_vio->inst,
                         array::begin(output.layers_entry_offset),
                         sizeof(uint32_t),
                         array::size(output.layers_entry_offset));

        build_vio->write(build_vio->inst,
                         array::begin(output.layers_localresource_count),
                         sizeof(uint32_t),
                         array::size(output.layers_localresource_count));

        build_vio->write(build_vio->inst,
                         array::begin(output.layers_localresource_offset),
                         sizeof(uint32_t),
                         array::size(output.layers_localresource_offset));

        build_vio->write(build_vio->inst,
                         array::begin(output.entry_data_offset),
                         sizeof(uint32_t),
                         array::size(output.entry_data_offset));

        build_vio->write(build_vio->inst, array::begin(output.layers_entry),
                         sizeof(layer_entry_t),
                         array::size(output.layers_entry));

        build_vio->write(build_vio->inst, array::begin(output.viewport),
                         sizeof(viewport_entry_t),
                         array::size(output.viewport));

        auto *blob = output.blob;
        build_vio->write(build_vio->inst, blob->data(blob->inst),
                         sizeof(uint8_t), blob->size(blob->inst));

        output.global_resource.destroy();
        output.layer_names.destroy();
        output.layers_entry_count.destroy();
        output.layers_entry_offset.destroy();
        output.entry_data_offset.destroy();
        output.layers_entry.destroy();
        output.viewport.destroy();
        output.local_resource.destroy();
        output.layers_localresource_count.destroy();
        output.layers_localresource_offset.destroy();
        ct_blob_a0.destroy(output.blob);

        CEL_FREE(ct_memory_a0.main_allocator(), source_data);
        return 1;
    }

    int init(struct ct_api_a0 *api) {
#ifdef CETECH_DEVELOP
        ct_resource_a0.compiler_register(
                ct_hash_a0.id64_from_str("render_config"),
                compiler);
#endif

        ct_renderer_a0.get_size(&_G.size_width, &_G.size_height);
        return 1;
    }
}

void on_update(float dt) {
    ct_event_header *event = ct_machine_a0.event_begin();

    ct_window_resized_event *ev;
    while (event != ct_machine_a0.event_end()) {
        switch (event->type) {
            case EVENT_WINDOW_RESIZED:
                ev = (ct_window_resized_event *) event;
                _G.need_reset = 1;
                _G.size_width = ev->width;
                _G.size_height = ev->height;
                break;

            default:
                break;
        }

        event = ct_machine_a0.event_next(event);
    }
}

void on_render() {
    if (_G.need_reset) {
        _G.need_reset = 0;
        recreate_all_viewport();
    }

    viewid_counter = 0;
}

void register_pass_compiler(uint64_t type,
                            ct_viewport_pass_compiler compiler) {
    map::set(_G.compiler_map, type, compiler);
}


struct fullscree_pass_data {
    uint8_t input_count;
    char input_name[8][32];
    uint64_t input_resource[8];
};

namespace viewport_module {
    static ct_viewport_a0 viewport_api = {
            .render_world = renderer_render_world,
            .register_layer_pass = renderer_register_layer_pass,
            .get_global_resource = renderer_get_global_resource,
            .create = renderer_create_viewport,
            .get_local_resource = render_viewport_get_local_resource,
            .resize = resize_viewport,
            .register_pass_compiler = register_pass_compiler
    };

    void _init_api(struct ct_api_a0 *api) {
        api->register_api("ct_viewport_a0", &viewport_api);
    }

    void _init(struct ct_api_a0 *api) {
        _init_api(api);

        ct_api_a0 = *api;

        _G = (struct G) {};


        GConfig = {
                .render_config_name = ct_config_a0.new_str("renderer.config",
                                                           "Render condfig",
                                                           "default")
        };

        _G.global_resource.init(ct_memory_a0.main_allocator());
        _G.on_pass.init(ct_memory_a0.main_allocator());

        _G.viewport_instance_map.init(ct_memory_a0.main_allocator());
        _G.viewport_instances.init(ct_memory_a0.main_allocator());
        _G.viewport_handler.init(ct_memory_a0.main_allocator());
        _G.compiler_map.init(ct_memory_a0.main_allocator());

        _G.type = ct_hash_a0.id64_from_str("render_config");
        ct_resource_a0.register_type(_G.type,
                                     renderconfig_resource::callback);


#ifdef CETECH_CAN_COMPILE
        renderconfig_compiler::init(api);
#endif

        ct_app_a0.register_on_render(on_render);
        ct_app_a0.register_on_update(on_update);
    }

    void _shutdown() {
        ct_cvar daemon = ct_config_a0.find("daemon");
        if (!ct_config_a0.get_int(daemon)) {
            ct_app_a0.unregister_on_render(on_render);
            ct_app_a0.unregister_on_update(on_update);

            _G.global_resource.destroy();
            _G.on_pass.destroy();

            _G.viewport_instance_map.destroy();
            _G.viewport_instances.destroy();
            _G.viewport_handler.destroy();

            _G.compiler_map.destroy();
        }

        _G = (struct G) {};
    }

}

CETECH_MODULE_DEF(
        viewport,
        {
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_mouse_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_app_a0);
            CETECH_GET_API(api, ct_machine_a0);
            CETECH_GET_API(api, ct_material_a0);
            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_mesh_renderer_a0);
            CETECH_GET_API(api, ct_window_a0);
            CETECH_GET_API(api, ct_blob_a0);
        },
        {
            viewport_module::_init(api);
        },
        {
            CEL_UNUSED(api);

            viewport_module::_shutdown();
        }
)