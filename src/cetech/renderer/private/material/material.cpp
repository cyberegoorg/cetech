//==============================================================================
// Include
//==============================================================================

#include <bgfx/bgfx.h>

#include "celib/allocator.h"
#include "celib/map.inl"
#include "celib/handler.inl"

#include "cetech/hashlib/hashlib.h"
#include "cetech/os/memory.h"
#include "cetech/api/api_system.h"


#include "cetech/machine/machine.h"
#include "cetech/resource/resource.h"

#include <cetech/renderer/renderer.h>
#include <cetech/renderer/material.h>
#include <cetech/renderer/shader.h>

#include <cetech/renderer/texture.h>
#include <cetech/module/module.h>
#include <celib/fmath.h>

#include "cetech/os/path.h"
#include "cetech/os/vio.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_texture_a0);
CETECH_DECL_API(ct_shader_a0);

using namespace celib;

#include "material.h"

#include "material_blob.h"

int materialcompiler_init(ct_api_a0 *api);

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "material"

#define _get_material_instance(idx) (_G.instance_data[_G.instance_offset[(idx)]])

//======================================1========================================
// GLobals
//==============================================================================

struct material_instance {
    const material_blob::blob_t *data;
    bgfx::UniformHandle *handlers;
};

static struct MaterialGlobals {
    Handler<uint32_t> material_handler;

    Map<uint32_t> instace_map;
    Map<uint32_t> resource_map;
    material_instance *material_instances;

    uint64_t type;
    cel_alloc *allocator;
} _G;


static void _destroy_instance(struct material_instance *instance) {
    const uint32_t n = instance->data->all_uniform_count;

    for (int i = 0; i < n; ++i) {
        bgfx::destroy(instance->handlers[i]);
    }

    CEL_FREE(ct_memory_a0.main_allocator(), instance->handlers);
}

static struct material_instance *_new_material(uint64_t name,
                                               uint32_t handler) {
    uint32_t idx = cel_array_size(_G.material_instances);
    cel_array_push(_G.material_instances, {}, _G.allocator);

    material_instance *instance = &_G.material_instances[idx];

    map::set(_G.instace_map, handler, idx);
    multi_map::insert(_G.resource_map, name, idx);

    return instance;
}

static struct material_instance *get_material_instance(ct_material material) {
    uint32_t idx = map::get(_G.instace_map, material.idx, UINT32_MAX);

    if (UINT32_MAX == idx) {
        return NULL;
    }

    return &_G.material_instances[idx];
}

static void spawn_material_instance(cel_alloc *alloc,
                                    const material_blob::blob_t *resource,
                                    material_instance *instance) {
    bgfx::UniformHandle *handlers = CEL_ALLOCATE(alloc,
                                                 bgfx::UniformHandle,
                                                 sizeof(bgfx::UniformHandle) *
                                                 resource->all_uniform_count);

    auto u_names = material_blob::uniform_names(resource);
    auto uniforms = material_blob::uniforms(resource);

    for (uint32_t i = 0; i < resource->all_uniform_count; ++i) {
        bgfx::UniformType::Enum ut = bgfx::UniformType::Count;

        switch (uniforms[i].type) {
            case MAT_VAR_NONE:
                break;

            case MAT_VAR_INT:
                ut = bgfx::UniformType::Int1;
                break;

            case MAT_VAR_TEXTURE:
            case MAT_VAR_TEXTURE_HANDLER:
                ut = bgfx::UniformType::Int1;
                break;

            case MAT_VAR_VEC4:
                ut = bgfx::UniformType::Vec4;
                break;

            case MAT_VAR_MAT44:
                ut = bgfx::UniformType::Mat4;
                break;
        }

        handlers[i] = bgfx::createUniform(&u_names[i * 32], ut, 1);
    }

    instance->data = resource;
    instance->handlers = handlers;

}

void reload_level_instance(uint64_t name,
                           void *data) {
    auto it = multi_map::find_first(_G.resource_map, name);
    while (it != nullptr) {
        struct material_instance *instance = &_G.material_instances[it->value];

        _destroy_instance(instance);

        auto *resource = material_blob::get(data);

        spawn_material_instance(ct_memory_a0.main_allocator(),
                                resource, instance);

        it = multi_map::find_next(_G.resource_map, it);
    }
}

//==============================================================================
// Resource
//==============================================================================
//    static const bgfx::ProgramHandle null_program = {};

static void *loader(ct_vio *input,
                    cel_alloc *allocator) {
    const int64_t size = input->size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    input->read(input, data, 1, size);
    return data;
}

static void unloader(void *new_data,
                     cel_alloc *allocator) {
    CEL_FREE(allocator, new_data);
}

static void online(uint64_t name,
                   void *data) {
    CEL_UNUSED(name, data);

}

static void offline(uint64_t name,
                    void *data) {
    CEL_UNUSED(name, data);
}

static void *reloader(uint64_t name,
                      void *old_data,
                      void *new_data,
                      cel_alloc *allocator) {
    offline(name, old_data);
    online(name, new_data);

    reload_level_instance(name, new_data);

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


//==============================================================================
// Interface
//==============================================================================
//    static const ct_material null_material = {};

static struct ct_material create(uint64_t name) {
    auto res = ct_resource_a0.get(_G.type, name);
    auto resource = material_blob::get(res);

    uint32_t h = handler::create(_G.material_handler);
    material_instance *instance = _new_material(name, h);


    cel_alloc *alloc = ct_memory_a0.main_allocator();

    spawn_material_instance(alloc, resource, instance);

    return (ct_material) {.idx=h};
}

static uint32_t _find_uniform_slot(const material_blob::blob_t *resource,
                                   const char *name) {

    const char *u_names = material_blob::uniform_names(resource);
    for (uint32_t i = 0; i < resource->all_uniform_count; ++i) {
        if (strcmp(&u_names[i * 32], name) != 0) {
            continue;
        }

        return i;
    }

    return UINT32_MAX;
}

static uint32_t _find_layer_slot(const material_blob::blob_t *resource,
                                 uint64_t layer) {

    auto *u_names = material_blob::layer_names(resource);
    for (uint32_t i = 0; i < resource->layer_count; ++i) {
        if (u_names[i] != layer) {
            continue;
        }

        return i;
    }

    return UINT32_MAX;
}

static void set_texture_handler(struct ct_material material,
                                const char *slot,
                                ct_texture texture) {
    material_instance *mat_inst = get_material_instance(material);

    auto *resource = mat_inst->data;
    auto *uniforms = material_blob::uniforms(resource);

    int slot_idx = _find_uniform_slot(resource, slot);
    uniforms[slot_idx].th = texture.idx;
    uniforms[slot_idx].type = MAT_VAR_TEXTURE_HANDLER;
}

static void set_texture(ct_material material,
                        const char *slot,
                        uint64_t texture) {
    material_instance *mat_inst = get_material_instance(material);

    auto *resource = mat_inst->data;
    auto *uniforms = material_blob::uniforms(resource);
    int slot_idx = _find_uniform_slot(resource, slot);
    uniforms[slot_idx].t = texture;
}


static void set_mat44f(ct_material material,
                       const char *slot,
                       float *value) {
    material_instance *mat_inst = get_material_instance(material);

    auto *resource = mat_inst->data;
    auto *uniforms = material_blob::uniforms(resource);

    int slot_idx = _find_uniform_slot(resource, slot);
    cel_mat4_move(uniforms[slot_idx].m44, value);
}

static void submit(ct_material material,
                   uint64_t layer,
                   uint8_t viewid) {
    material_instance *mat_inst = get_material_instance(material);

//        if(!mat_inst) {
//            return;
//        }

    auto *resource = mat_inst->data;
    auto *handlers = mat_inst->handlers;
    auto layer_idx = _find_layer_slot(resource, layer);

    if (UINT32_MAX == layer_idx) {
        return;
    }

    auto *shader_names = material_blob::shader_name(resource);
    auto *layer_offset = material_blob::layer_offset(resource);
    auto *uniforms = material_blob::uniforms(resource);
    auto *uniform_cout = material_blob::uniform_count(resource);
    auto *render_state = material_blob::render_state(resource);

    auto shader = ct_shader_a0.get(shader_names[layer_idx]);

    auto offset = layer_offset[layer_idx];

    uint8_t texture_stage = 0;

    for (uint32_t i = 0; i < uniform_cout[layer_idx]; ++i) {
        auto &uniform = uniforms[offset + i];
        auto type = uniform.type;

        switch (type) {
            case MAT_VAR_NONE:
                break;

            case MAT_VAR_INT:
                bgfx::setUniform(handlers[i], &uniform.i, 1);
                break;

            case MAT_VAR_TEXTURE: {
                auto texture = ct_texture_a0.get(uniform.t);
                bgfx::setTexture(texture_stage, handlers[i],
                                 {texture.idx});
                ++texture_stage;
            }
                break;

            case MAT_VAR_TEXTURE_HANDLER: {
                bgfx::setTexture(texture_stage, handlers[i], {uniform.th});
                ++texture_stage;
            }
                break;

            case MAT_VAR_VEC4:
                bgfx::setUniform(handlers[i], &uniform.v4, 1);
                break;

            case MAT_VAR_MAT44:
                bgfx::setUniform(handlers[i], &uniform.m44, 1);
                break;
        }
    }

    uint64_t state = render_state[layer_idx];
    bgfx::setState(state, 0);
    bgfx::submit(viewid, {shader.idx});
}

static struct ct_material_a0 material_api = {
        .resource_create = create,
        .set_texture = set_texture,
        .set_texture_handler = set_texture_handler,
        .set_mat44f = set_mat44f,
        .submit = submit
};

static int init(ct_api_a0 *api) {
    _G = {
            .allocator = ct_memory_a0.main_allocator(),
    };
    api->register_api("ct_material_a0", &material_api);

    _G.type = CT_ID64_0("material");
    _G.material_handler.init(ct_memory_a0.main_allocator());
    _G.instace_map.init(ct_memory_a0.main_allocator());
    _G.resource_map.init(ct_memory_a0.main_allocator());

    ct_resource_a0.register_type(_G.type, callback);

    materialcompiler_init(api);

    return 1;
}

static void shutdown() {
    _G.material_handler.destroy();
    _G.instace_map.destroy();
    _G.resource_map.destroy();

    cel_array_free(_G.material_instances, _G.allocator);

}

CETECH_MODULE_DEF(
        material,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_texture_a0);
            CETECH_GET_API(api, ct_shader_a0);
        },
        {
            CEL_UNUSED(reload);
            init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            shutdown();
        }
)