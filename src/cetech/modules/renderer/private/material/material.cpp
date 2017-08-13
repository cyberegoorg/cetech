//==============================================================================
// Include
//==============================================================================

#include <bgfx/bgfx.h>

#include "celib/allocator.h"
#include "celib/array.inl"
#include "celib/map.inl"
#include "celib/handler.inl"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"


#include "cetech/engine/machine/machine.h"
#include "cetech/engine/resource/resource.h"
#include "cetech/engine/entity/entity.h"

#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/renderer/private/texture/texture.h>
#include <cetech/modules/renderer/private/shader/shader.h>
#include <celib/fpumath.h>

#include "cetech/core/os/path.h"
#include "cetech/core/os/vio.h"

#include "material.h"

#include "material_blob.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);

using namespace celib;

namespace material_compiler {
    int init(ct_api_a0 *api);
}

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "material"

#define _get_material_instance(idx) (_G.instance_data[_G.instance_offset[(idx)]])

//======================================1========================================
// GLobals
//==============================================================================
static struct MaterialGlobals {
    Map<uint32_t> instace_map;
    Array<uint32_t> instance_offset;
    Array<uint8_t> instance_data;
    Array<bgfx::UniformHandle> instance_uniform_data;
    Handler<uint32_t> material_handler;

    uint64_t type;

    void init(cel_alloc *allocator) {
        CEL_UNUSED(allocator);

        this->type = ct_hash_a0.id64_from_str("material");
        this->material_handler.init(ct_memory_a0.main_allocator());
        this->instace_map.init(ct_memory_a0.main_allocator());
        this->instance_offset.init(ct_memory_a0.main_allocator());
        this->instance_data.init(ct_memory_a0.main_allocator());
        this->instance_uniform_data.init(ct_memory_a0.main_allocator());
    }

    void shutdown() {
        this->material_handler.destroy();
        this->instace_map.destroy();
        this->instance_offset.destroy();
        this->instance_data.destroy();
        this->instance_uniform_data.destroy();
    }
} _G;


//==============================================================================
// Resource
//==============================================================================
namespace material_resource {
//    static const bgfx::ProgramHandle null_program = {};

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
        CEL_UNUSED(name, data);

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
            .loader = material_resource::loader,
            .unloader = material_resource::unloader,
            .online = material_resource::online,
            .offline = material_resource::offline,
            .reloader = material_resource::reloader
    };
}


//==============================================================================
// Interface
//==============================================================================


namespace material {
    int init(ct_api_a0 *api) {
        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_resource_a0);
        CETECH_GET_API(api, ct_path_a0);
        CETECH_GET_API(api, ct_vio_a0);
        CETECH_GET_API(api, ct_hash_a0);

        _G.init(ct_memory_a0.main_allocator());

        ct_resource_a0.register_type(_G.type, material_resource::callback);

#ifdef CETECH_CAN_COMPILE
        material_compiler::init(api);
#endif
        return 1;
    }

    void shutdown() {
        _G.shutdown();
    }

//    static const ct_material null_material = {};

    ct_material create(uint64_t name) {
        auto resource = material_blob::get(ct_resource_a0.get(_G.type, name));

        uint32_t size = material_blob::blob_size(resource);
        uint32_t h = handler::create(_G.material_handler);

        uint32_t idx = (uint32_t) array::size(_G.instance_offset);

        map::set(_G.instace_map, h, idx);

        uint32_t offset = array::size(_G.instance_data);
        array::push(_G.instance_data, (uint8_t *) resource, size);
        array::push_back(_G.instance_offset, offset);

        bgfx::UniformHandle bgfx_uniforms[resource->all_uniform_count];
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
                    ut = bgfx::UniformType::Int1;
                    break;
                case MAT_VAR_VEC4:
                    ut = bgfx::UniformType::Vec4;
                    break;
                case MAT_VAR_MAT44:
                    ut = bgfx::UniformType::Mat4;
                    break;
            }

            bgfx_uniforms[i] = bgfx::createUniform(&u_names[i * 32], ut, 1);
        }

        array::push(_G.instance_uniform_data, bgfx_uniforms,
                    resource->all_uniform_count);

        return (ct_material) {.idx=h};
    }

    uint32_t _find_uniform_slot(const material_blob::blob_t *resource,
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

    uint32_t _find_layer_slot(const material_blob::blob_t *resource,
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

    void set_texture(ct_material material,
                     const char *slot,
                     uint64_t texture) {

        uint32_t idx = map::get(_G.instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        auto *resource = material_blob::get(&_get_material_instance(idx));
        auto *uniforms = material_blob::uniforms(resource);

        int slot_idx = _find_uniform_slot(resource, slot);
        uniforms[slot_idx].t = texture;
    }


    void set_mat44f(ct_material material,
                    const char *slot,
                    float *value) {
        uint32_t idx = map::get(_G.instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        auto *resource = material_blob::get(&_get_material_instance(idx));
        auto *uniforms = material_blob::uniforms(resource);

        int slot_idx = _find_uniform_slot(resource, slot);
        celib::mat4_move(uniforms[slot_idx].m44, value);
    }

    void submit(ct_material material,
                uint64_t layer, uint8_t viewid) {
        uint32_t idx = map::get(_G.instace_map, material.idx, UINT32_MAX);

        if (UINT32_MAX == idx) {
            return;
        }

        auto resource = material_blob::get(&_get_material_instance(idx));
        auto layer_idx = _find_layer_slot(resource, layer);

        if(UINT32_MAX == layer_idx) {
            return;
        }

        auto *shader_names = material_blob::shader_name(resource);
        auto *layer_offset = material_blob::layer_offset(resource);
        auto *uniforms = material_blob::uniforms(resource);
        auto *uniform_cout = material_blob::uniform_count(resource);

        auto shader_name = shader::shader_get(shader_names[layer_idx]);

        auto offset = layer_offset[layer_idx];
        bgfx::UniformHandle *u_handler = &_G.instance_uniform_data[offset];

        uint8_t texture_stage = 0;

        for (uint32_t i = 0; i < uniform_cout[layer_idx]; ++i) {
            auto &uniform = uniforms[offset + i];
            auto type = uniform.type;

            switch (type) {
                case MAT_VAR_NONE:
                    break;
                case MAT_VAR_INT:
                    bgfx::setUniform(u_handler[i], &uniform.i, 1);
                    break;

                case MAT_VAR_TEXTURE: {
                    auto texture = texture::texture_get(uniform.t);
                    bgfx::setTexture(texture_stage, u_handler[i], texture);
                    ++texture_stage;
                }
                    break;
                case MAT_VAR_VEC4:
                    bgfx::setUniform(u_handler[i], &uniform.v4, 1);
                    break;
                case MAT_VAR_MAT44:
                    bgfx::setUniform(u_handler[i], &uniform.m44, 1);
                    break;
            }
        }

        uint64_t state = (0
                          | BGFX_STATE_RGB_WRITE
                          | BGFX_STATE_ALPHA_WRITE
                          | BGFX_STATE_DEPTH_TEST_LESS
                          | BGFX_STATE_DEPTH_WRITE
                          | BGFX_STATE_CULL_CCW
                          | BGFX_STATE_MSAA
        );

        bgfx::setState(state, 0);
        bgfx::submit(viewid, shader_name);
    }

}