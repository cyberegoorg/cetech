//==============================================================================
// includes
//==============================================================================
#include <cstdio>

#include <cetech/core/memory/allocator.h>
#include <cetech/core/containers/map.inl>
#include <cetech/core/containers/handler.h>


#include <cetech/core/api/api_system.h>
#include <cetech/engine/config/config.h>
#include <cetech/macros.h>
#include <cetech/core/module/module.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/os/vio.h>

#include <cetech/engine/application/application.h>
#include <cetech/core/os/window.h>
#include <cetech/engine/entity/entity.h>
#include <cetech/engine/input/input.h>
#include <cetech/engine/resource/resource.h>

#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/renderer/texture.h>
#include <cetech/engine/camera/camera.h>
#include <cetech/engine/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/machine/machine.h>
#include <cetech/engine/renderer/mesh_renderer.h>

#include "bgfx/platform.h"

#include <cetech/engine/renderer/viewport.h>
#include <cetech/core/coredb/coredb.h>
#include <cetech/core/containers/hash.h>
#include "cetech/engine/renderer/scene.h"
#include "cetech/engine/renderer/material.h"

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
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_cdb_a0);

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


#define _G ViewportsGlobals
static struct G {
    Map<bgfx::TextureHandle> global_resource;
    Map<ct_viewport_on_pass_t> on_pass;
    ct_hash_t viewport_instance_map;
    viewport_instance *viewport_instances;
    ct_handler_t viewport_handler;

    Map<ct_viewport_pass_compiler> compiler_map;

    uint64_t type;

    uint32_t size_width;
    uint32_t size_height;
    int need_reset;
    ct_cdb_obj_t *config;
    ct_alloc *allocator;
} _G = {};

#define CONFIG_RENDER_CONFIG CT_ID64_0("default")
#define CONFIG_DAEMON CT_ID64_0("daemon")

#define VIEWPORT_PROP CT_ID64_0("viewport")

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

static void renderer_render_world(ct_world world,
                                  ct_camera camera,
                                  ct_viewport viewport) {

    auto idx = ct_hash_lookup(&_G.viewport_instance_map, viewport.idx,
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


static ct_texture get_local_resource(viewport_instance &instance,
                                     uint64_t name) {
    for (uint32_t i = 0; i < instance.resource_count; ++i) {
        if (instance.local_resource_name[i] != name) {
            continue;
        }

        return {instance.local_resource[i]};
    }

    return {UINT16_MAX};
}

static struct ct_texture
render_viewport_get_local_resource(ct_viewport viewport,
                                   uint64_t name) {

    auto idx = ct_hash_lookup(&_G.viewport_instance_map, viewport.idx,
                               UINT32_MAX);
    auto &vi = _G.viewport_instances[idx];

    return get_local_resource(vi, name);
}

static void _init_viewport(viewport_instance &vi,
                           uint64_t name,
                           float width,
                           float height) {
    const char *render_config = ct_cdb_a0.read_str(
            ct_config_a0.config_object(), CT_ID64_0("renderer.config"), "");

    ct_cdb_obj_t* object = ct_resource_a0.get_obj(_G.type, CT_ID64_0(render_config));
    void* resource = ct_cdb_a0.read_ptr(object, VIEWPORT_PROP, NULL);
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

            for (uint32_t k = 0; k < localresource_count; ++k) {
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
            for (uint32_t k = 0; k < entry_count; ++k) {
                auto &e = entry[k];

                bgfx::TextureHandle th[e.output_count];
                for (int l = 0; l < e.output_count; ++l) {
                    // TODO if not found in local then find in global
                    th[l] = {get_local_resource(vi, e.output[l]).idx};
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

static void resize_viewport(ct_viewport viewport,
                            float width,
                            float height) {

    auto idx = ct_hash_lookup(&_G.viewport_instance_map, viewport.idx,
                               UINT32_MAX);
    auto &vi = _G.viewport_instances[idx];

    if ((width != vi.size[0]) || (height != vi.size[1])) {
        _init_viewport(vi, vi.viewport, width, height);
    }
}

static void recreate_all_viewport() {
    for (uint32_t i = 0; i < ct_array_size(_G.viewport_instances); ++i) {
        auto &vi = _G.viewport_instances[i];
        _init_viewport(vi, vi.viewport, vi.size[0], vi.size[1]);
    }
}

static ct_viewport renderer_create_viewport(uint64_t name,
                                            float width,
                                            float height) {
    viewport_instance vi = {};
    _init_viewport(vi, name, width, height);

    auto idx = ct_array_size(_G.viewport_instances);
    uint64_t id = ct_handler_create(&_G.viewport_handler, _G.allocator);
    ct_hash_add(&_G.viewport_instance_map, id, idx, _G.allocator);
    ct_array_push(_G.viewport_instances, vi, _G.allocator);

    return {.idx = id};
}


//==============================================================================
// render_config resource
//==============================================================================


static void online(uint64_t name,
                   struct ct_vio* input, struct ct_cdb_obj_t* obj) {
    CT_UNUSED(name);

    const uint64_t size = input->size(input);
    char *data = CT_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

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

        auto h = bgfx::createTexture2D(ration, false, 1, format, samplerFlags);

        map::set(_G.global_resource, gr.name, h);
    }

    ct_cdb_writer_t* writer = ct_cdb_a0.write_begin(obj);
    ct_cdb_a0.set_ptr(writer, VIEWPORT_PROP, data);
    ct_cdb_a0.write_commit(writer);
}

static void offline(uint64_t name,
                    struct ct_cdb_obj_t* obj) {
    CT_UNUSED(name, obj);
    //auto *blob = renderconfig_blob::get(data);


}

static const ct_resource_type_t callback = {
        .online = online,
        .offline = offline,
};

//// COMPIELr
struct compiler_output {
    uint64_t *layer_names;
    uint32_t *layers_entry_count;
    uint32_t *layers_entry_offset;
    uint32_t *layers_localresource_count;
    uint32_t *layers_localresource_offset;
    uint32_t *entry_data_offset;
    render_resource_t *global_resource;
    render_resource_t *local_resource;
    layer_entry_t *layers_entry;
    viewport_entry_t *viewport;
    char *blob;
};

static void compile_global_resource(uint32_t idx,
                                    struct ct_yamlng_node value,
                                    void *_data) {

    CT_UNUSED(idx);

    render_resource_t gs = {};

    compiler_output &output = *((compiler_output *) _data);
    ct_yng_doc *d = value.d;

    ////////////////////////////////////////////////////
    uint64_t keys[2] = {
            d->hash(d->inst, value),
            ct_yng_a0.calc_key("name")
    };
    uint64_t k = ct_yng_a0.combine_key(keys, CETECH_ARRAY_LEN(keys));
    const char *name_str = d->get_string(d->inst, k, "");
    gs.name = CT_ID64_0(name_str);

    /////////////////////////////////////////////////////
    keys[1] = ct_yng_a0.calc_key("type");
    k = ct_yng_a0.combine_key(keys, CETECH_ARRAY_LEN(keys));
    const char *type_str = d->get_string(d->inst, k, "");
    gs.type = CT_ID64_0(type_str);

    /////////////////////////////////////////////////////
    keys[1] = ct_yng_a0.calc_key("format");
    k = ct_yng_a0.combine_key(keys, CETECH_ARRAY_LEN(keys));
    const char *format_str = d->get_string(d->inst, k, "");
    gs.format = CT_ID64_0(format_str);

    /////////////////////////////////////////////////////
    keys[1] = ct_yng_a0.calc_key("ration");
    k = ct_yng_a0.combine_key(keys, CETECH_ARRAY_LEN(keys));
    const char *ration_str = d->get_string(d->inst, k, "");
    gs.ration = CT_ID64_0(ration_str);

    ct_array_push(output.global_resource, gs, _G.allocator);
}


static void compile_layer_entry(uint32_t idx,
                                struct ct_yamlng_node value,
                                void *_data) {
    CT_UNUSED(idx);

    layer_entry_t le = {};

    auto &output = *((compiler_output *) _data);
    ct_yng_doc *d = value.d;

    ////////////////////////////////////////////////////
    uint64_t keys[2] = {
            d->hash(d->inst, value),
            ct_yng_a0.calc_key("name")
    };
    uint64_t k = ct_yng_a0.combine_key(keys, CETECH_ARRAY_LEN(keys));
    const char *name_str = d->get_string(d->inst, k, "");
    le.name = CT_ID64_0(name_str);


    /////////////////////////////////////////////////////
    keys[1] = ct_yng_a0.calc_key("type");
    k = ct_yng_a0.combine_key(keys, CETECH_ARRAY_LEN(keys));
    const char *type_str = d->get_string(d->inst, k, "");
    le.type = CT_ID64_0(type_str);

    /////////////////////////////////////////////////////
    keys[1] = ct_yng_a0.calc_key("output");
    k = ct_yng_a0.combine_key(keys, CETECH_ARRAY_LEN(keys));

    ct_yamlng_node output_node = d->get(d->inst, k);
    if (0 != output_node.idx) {
        d->foreach_seq_node(
                d->inst,
                output_node,
                [](uint32_t idx,
                   struct ct_yamlng_node value,
                   void *_data) {
                    auto &le = *((layer_entry_t *) _data);
                    ct_yng_doc *d = value.d;

                    const char *output_name = d->as_string(d->inst, value, "");
                    le.output[idx] = CT_ID64_0(output_name);

                    ++le.output_count;
                }, &le);
    }

    auto compiler = map::get<ct_viewport_pass_compiler>(_G.compiler_map,
                                                        le.type, NULL);

    ct_array_push(output.entry_data_offset, ct_array_size(output.blob),
                   _G.allocator);

    if (compiler) {
        compiler(value, &output.blob);
    }

    ct_array_push(output.layers_entry, le, _G.allocator);

}

static void compile_layers(struct ct_yamlng_node key,
                           struct ct_yamlng_node value,
                           void *_data) {
    ct_yng_doc *d = key.d;
    compiler_output &output = *((compiler_output *) _data);

    const char *name_str = d->as_string(d->inst, key, "");

    auto name_id = CT_ID64_0(name_str);
    auto layer_offset = ct_array_size(output.layers_entry);

    ct_array_push(output.layer_names, name_id, _G.allocator);
    ct_array_push(output.layers_entry_offset, layer_offset, _G.allocator);

    d->foreach_seq_node(d->inst, value, compile_layer_entry, &output);

    const uint32_t entry_count = ct_array_size(output.layers_entry);

    ct_array_push(output.layers_entry_count, entry_count - layer_offset,
                   _G.allocator);
}

static void compiler(const char *filename,
                     char **output_blob,
                     struct ct_compilator_api *compilator_api) {

    CT_UNUSED(filename);
    CT_UNUSED(compilator_api);

    struct compiler_output output = {
    };

    ct_yng_doc *doc = ct_ydb_a0.get(filename);

    //==================================================================
    // Global resource
    //==================================================================
    ct_yamlng_node global_resource = doc->get(doc->inst,
                                              ct_yng_a0.calc_key(
                                                      "global_resource"));
    if (0 != global_resource.idx) {
        doc->foreach_seq_node(doc->inst, global_resource,
                              compile_global_resource, &output);
    }

    //==================================================================
    // layers
    //==================================================================
    ct_yamlng_node layers = doc->get(doc->inst,
                                     ct_yng_a0.calc_key("layers"));
    if (0 != layers.idx) {
        doc->foreach_dict_node(doc->inst, layers, compile_layers, &output);
    }

    //==================================================================
    // Viewport
    //==================================================================
    ct_yamlng_node viewport = doc->get(doc->inst,
                                       ct_yng_a0.calc_key("viewport"));
    if (0 != viewport.idx) {
        doc->foreach_dict_node(
                doc->inst,
                viewport,
                [](struct ct_yamlng_node key,
                   struct ct_yamlng_node value,
                   void *_data) {
                    ct_yng_doc *d = key.d;
                    compiler_output &output = *((compiler_output *) _data);

                    const char *name_str = d->as_string(d->inst, key, "");
                    auto name_id = CT_ID64_0(name_str);


                    uint64_t keys[2] = {
                            d->hash(d->inst, value),
                            ct_yng_a0.calc_key("layers")
                    };
                    uint64_t k = ct_yng_a0.combine_key(keys,
                                                       CETECH_ARRAY_LEN(
                                                               keys));
                    const char *layers_str = d->get_string(d->inst, k, "");
                    auto layers_id = CT_ID64_0(layers_str);
                    viewport_entry_t ve = {};

                    ve.name = name_id;
                    ve.layer = layers_id;

                    ct_array_push(output.viewport, ve, _G.allocator);

                    //////
                    auto localresource_offset = ct_array_size(
                            output.local_resource);

                    ct_array_push(
                            output.layers_localresource_offset,
                            localresource_offset, _G.allocator);

                    keys[1] = ct_yng_a0.calc_key("local_resource");
                    k = ct_yng_a0.combine_key(keys,
                                              CETECH_ARRAY_LEN(keys));
                    ct_yamlng_node local_resource = d->get(d->inst, k);
                    if (0 != local_resource.idx) {
                        d->foreach_seq_node(
                                d->inst,
                                local_resource,
                                [](uint32_t idx,
                                   struct ct_yamlng_node value,
                                   void *_data) {

                                    CT_UNUSED(idx);

                                    render_resource_t gs = {};

                                    compiler_output &output = *((compiler_output *) _data);
                                    ct_yng_doc *d = value.d;

                                    ////////////////////////////////////////////////////
                                    uint64_t keys[2] = {
                                            d->hash(d->inst, value),
                                            ct_yng_a0.calc_key("name")
                                    };
                                    uint64_t k = ct_yng_a0.combine_key(
                                            keys, CETECH_ARRAY_LEN(keys));
                                    const char *name_str = d->get_string(
                                            d->inst, k, "");
                                    gs.name = CT_ID64_0(name_str);

                                    /////////////////////////////////////////////////////
                                    keys[1] = ct_yng_a0.calc_key("type");
                                    k = ct_yng_a0.combine_key(keys,
                                                              CETECH_ARRAY_LEN(
                                                                      keys));
                                    const char *type_str = d->get_string(
                                            d->inst, k, "");
                                    gs.type = CT_ID64_0(type_str);


                                    /////////////////////////////////////////////////////
                                    keys[1] = ct_yng_a0.calc_key(
                                            "format");
                                    k = ct_yng_a0.combine_key(keys,
                                                              CETECH_ARRAY_LEN(
                                                                      keys));
                                    const char *format_str = d->get_string(
                                            d->inst, k, "");
                                    gs.format = CT_ID64_0(format_str);

                                    /////////////////////////////////////////////////////
                                    keys[1] = ct_yng_a0.calc_key("ration");
                                    k = ct_yng_a0.combine_key(keys,
                                                              CETECH_ARRAY_LEN(
                                                                      keys));
                                    const char *ration_str = d->get_string(
                                            d->inst, k, "");
                                    gs.ration = CT_ID64_0(ration_str);


                                    ct_array_push(
                                            output.local_resource, gs,
                                            _G.allocator);
                                }, &output);

                        ct_array_push(
                                output.layers_localresource_count,
                                ct_array_size(output.local_resource) -
                                localresource_offset, _G.allocator);
                    }

                }, &output);
    }

    renderconfig_blob::blob_t resource = {
            .global_resource_count = ct_array_size(
                    output.global_resource),

            .layer_count = ct_array_size(output.layer_names),

            .layer_entry_count = ct_array_size(output.layers_entry),

            .local_resource_count = ct_array_size(
                    output.local_resource),

            .viewport_count = ct_array_size(output.viewport),
    };


    ct_array_push_n(*output_blob, &resource, sizeof(resource), _G.allocator);
    ct_array_push_n(*output_blob,
                     output.global_resource,
                     sizeof(render_resource_t) *
                     ct_array_size(output.global_resource), _G.allocator);

    ct_array_push_n(*output_blob,
                     output.local_resource,
                     sizeof(render_resource_t) *
                     ct_array_size(output.local_resource), _G.allocator);

    ct_array_push_n(*output_blob,
                     output.layer_names,
                     sizeof(uint64_t) *
                     ct_array_size(output.layer_names), _G.allocator);

    ct_array_push_n(*output_blob,
                     output.layers_entry_count,
                     sizeof(uint32_t) *
                     ct_array_size(output.layers_entry_count), _G.allocator);

    ct_array_push_n(*output_blob,
                     output.layers_entry_offset,
                     sizeof(uint32_t) *
                     ct_array_size(output.layers_entry_offset), _G.allocator);

    ct_array_push_n(*output_blob,
                     output.layers_localresource_count,
                     sizeof(uint32_t) *
                     ct_array_size(output.layers_localresource_count),
                     _G.allocator);

    ct_array_push_n(*output_blob,
                     output.layers_localresource_offset,
                     sizeof(uint32_t) *
                     ct_array_size(output.layers_localresource_offset),
                     _G.allocator);

    ct_array_push_n(*output_blob,
                     output.entry_data_offset,
                     sizeof(uint32_t) *
                     ct_array_size(output.entry_data_offset), _G.allocator);

    ct_array_push_n(*output_blob, output.layers_entry,
                     sizeof(layer_entry_t) *
                     ct_array_size(output.layers_entry), _G.allocator);

    ct_array_push_n(*output_blob, output.viewport,
                     sizeof(viewport_entry_t) *
                     ct_array_size(output.viewport), _G.allocator);

    ct_array_push_n(*output_blob, output.blob,
                     sizeof(uint8_t) * ct_array_size(output.blob),
                     _G.allocator);

    ct_array_free(output.global_resource, _G.allocator);
    ct_array_free(output.layer_names, _G.allocator);
    ct_array_free(output.layers_entry_count, _G.allocator);
    ct_array_free(output.layers_entry_offset, _G.allocator);
    ct_array_free(output.entry_data_offset, _G.allocator);
    ct_array_free(output.layers_entry, _G.allocator);
    ct_array_free(output.viewport, _G.allocator);
    ct_array_free(output.local_resource, _G.allocator);
    ct_array_free(output.layers_localresource_count, _G.allocator);
    ct_array_free(output.layers_localresource_offset, _G.allocator);

    ct_array_free(output.blob, _G.allocator);
}

static int init(struct ct_api_a0 *api) {
    CT_UNUSED(api);

#ifdef CETECH_DEVELOP
    ct_resource_a0.compiler_register(
            CT_ID64_0("render_config"), compiler, true);
#endif

    ct_renderer_a0.get_size(&_G.size_width, &_G.size_height);
    return 1;
}

static void on_update(float dt) {
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

static void on_render() {
    if (_G.need_reset) {
        _G.need_reset = 0;
        recreate_all_viewport();
    }

    viewid_counter = 0;
}

static void register_pass_compiler(uint64_t type,
                                   ct_viewport_pass_compiler compiler) {
    map::set(_G.compiler_map, type, compiler);
}


struct fullscree_pass_data {
    uint8_t input_count;
    char input_name[8][32];
    uint64_t input_resource[8];
};

static ct_viewport_a0 viewport_api = {
        .render_world = renderer_render_world,
        .register_layer_pass = renderer_register_layer_pass,
        .get_global_resource = renderer_get_global_resource,
        .create = renderer_create_viewport,
        .get_local_resource = render_viewport_get_local_resource,
        .resize = resize_viewport,
        .register_pass_compiler = register_pass_compiler
};

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_viewport_a0", &viewport_api);
}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    ct_api_a0 = *api;

    _G = {
            .allocator = ct_memory_a0.main_allocator(),
            .type = CT_ID64_0("render_config"),
    };

    _G.allocator = ct_memory_a0.main_allocator();
    _G.config = ct_config_a0.config_object();

    ct_cdb_writer_t *writer = ct_cdb_a0.write_begin(_G.config);
    if (!ct_cdb_a0.prop_exist(_G.config, CONFIG_RENDER_CONFIG)) {
        ct_cdb_a0.set_string(writer, CONFIG_RENDER_CONFIG, "default");
    }
    ct_cdb_a0.write_commit(writer);

    _G.global_resource.init(ct_memory_a0.main_allocator());
    _G.on_pass.init(ct_memory_a0.main_allocator());
    _G.compiler_map.init(ct_memory_a0.main_allocator());

    ct_resource_a0.register_type(_G.type, callback);
    init(api);
    ct_renderer_a0.register_on_render(on_render);
    //ct_app_a0.register_on_update(on_update);
}

static void _shutdown() {
    if (!ct_cdb_a0.read_uint32(_G.config, CONFIG_DAEMON, 0)) {
        ct_renderer_a0.unregister_on_render(on_render);
        ct_app_a0.unregister_on_update(on_update);

        _G.global_resource.destroy();
        _G.on_pass.destroy();

        ct_hash_free(&_G.viewport_instance_map, _G.allocator);
        ct_array_free(_G.viewport_instances, _G.allocator);
        ct_handler_free(&_G.viewport_handler, _G.allocator);

        _G.compiler_map.destroy();
    }

    _G = (struct G) {};
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
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_cdb_a0);
        },
        {

            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            _shutdown();
        }
)