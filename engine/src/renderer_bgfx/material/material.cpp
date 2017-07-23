//==============================================================================
// Include
//==============================================================================

#include <bgfx/bgfx.h>

#include <celib/allocator.h>
#include <celib/array.inl>
#include <celib/map.inl>
#include <celib/handler.inl>

#include <cetech/hash.h>
#include <cetech/memory.h>
#include <cetech/machine/machine.h>
#include <cetech/api_system.h>
#include <cetech/os/errors.h>


#include <cetech/resource.h>
#include <cetech/entity.h>
#include <cetech/renderer.h>

#include "material_blob.h"

#include "../texture/texture.h"
#include "../shader/shader.h"
#include <cetech/os/path.h>
#include <cetech/os/vio.h>

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

#define _get_resorce(idx) (_G.instance_data[_G.instance_offset[(idx)]])
#define material_blob_uniform_bgfx(r)    ((bgfx::UniformHandle*) ((material_blob::mat44f_value(r)+((r)->mat44f_count*16))))

//==============================================================================
// GLobals
//==============================================================================
namespace {
    static struct MaterialGlobals {
        Map<uint32_t> instace_map;
        Array<uint32_t> instance_offset;
        Array<uint8_t> instance_data;
        Array<uint32_t> instance_uniform_data;
        Handler<uint32_t> material_handler;
        uint64_t type;

        void init(cel_alloc *allocator) {
            this->type = ct_hash_a0.id64_from_str("material");
            this->material_handler.init(ct_memory_a0.main_allocator());
            this->instace_map.init(ct_memory_a0.main_allocator());
            this->instance_offset.init(ct_memory_a0.main_allocator());
            this->instance_data.init(ct_memory_a0.main_allocator());
        }

        void shutdown() {
            this->material_handler.destroy();
            this->instace_map.destroy();
            this->instance_offset.destroy();
            this->instance_data.destroy();
        }
    } _G;
}


//==============================================================================
// Resource
//==============================================================================
namespace material_resource {
    static const bgfx::ProgramHandle null_program = {0};

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

    }

    void offline(uint64_t name,
                 void *data) {
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

    static const ct_material null_material = {0};

    ct_material create(uint64_t name) {
        auto resource = material_blob::get(ct_resource_a0.get(_G.type, name));

        uint32_t size = material_blob::blob_size(resource);
        uint32_t h = handler::create(_G.material_handler);

        uint32_t idx = (uint32_t) array::size(_G.instance_offset);

        map::set(_G.instace_map, h, idx);

        uint32_t offset = array::size(_G.instance_data);
        array::push(_G.instance_data, (uint8_t *) resource, size);
        array::push_back(_G.instance_offset, offset);

        // write bgfx uniform handlers
        bgfx::UniformHandle bgfx_uniforms[resource->uniforms_count];
        const char *u_names = (const char *) (resource + 1);

        uint32_t off = 0;
        uint32_t tmp_off = 0;
        off += resource->texture_count;
        for (int i = 0; i < resource->texture_count; ++i) {
            bgfx_uniforms[i] = bgfx::createUniform(&u_names[i * 32],
                                                   bgfx::UniformType::Int1, 1);
        }

        tmp_off = off;
        off += resource->vec4f_count;
        for (int i = tmp_off; i < off; ++i) {
            bgfx_uniforms[i] = bgfx::createUniform(&u_names[i * 32],
                                                   bgfx::UniformType::Vec4, 1);
        }

        tmp_off = off;
        off += resource->mat33f_count;
        for (int i = tmp_off; i < off; ++i) {
            bgfx_uniforms[i] = bgfx::createUniform(&u_names[i * 32],
                                                   bgfx::UniformType::Mat3, 1);
        }

        tmp_off = off;
        off += resource->mat44f_count;
        for (int i = tmp_off; i < off; ++i) {
            bgfx_uniforms[i] = bgfx::createUniform(&u_names[i * 32],
                                                   bgfx::UniformType::Mat4, 1);
        }

        array::push(_G.instance_data, (uint8_t *) bgfx_uniforms,
                    sizeof(bgfx::UniformHandle) * resource->uniforms_count);

        return (ct_material) {.idx=h};
    }


    uint32_t get_texture_count(ct_material material) {
        uint32_t idx = map::get(_G.instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return 0;
        }

        auto resource = material_blob::get(&_get_resorce(idx));

        return material_blob::texture_count(resource);
    }

    uint32_t _material_find_slot(const material_blob::blob_t *resource,
                                 const char *name) {

        const char *u_names = material_blob::uniform_names(resource);
        for (uint32_t i = 0; i < resource->uniforms_count; ++i) {
            if (strcmp(&u_names[i * 32], name) != 0) {
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

        auto resource = material_blob::get(&_get_resorce(idx));

        uint64_t *u_texture = material_blob::texture_names(resource);

        int slot_idx = _material_find_slot(resource, slot);

        u_texture[slot_idx] = texture;
    }


    void set_mat44f(ct_material material,
                    const char *slot,
                    float *value) {
        uint32_t idx = map::get(_G.instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        auto resource = material_blob::get(&_get_resorce(idx));

        float *u_mat44f = material_blob::mat44f_value(resource);

        int slot_idx = _material_find_slot(resource, slot);


        memcpy(&u_mat44f[16*(slot_idx -
                         (resource->texture_count + resource->vec4f_count +
                          resource->mat33f_count))], value, sizeof(float) * 16);
    }


    void use(ct_material material) {
        uint32_t idx = map::get(_G.instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        auto resource = material_blob::get(&_get_resorce(idx));

        uint64_t *u_texture = material_blob::texture_names(resource);
        float *u_vec4f = material_blob::vec4f_value(resource);
        float *u_mat33f = material_blob::mat33f_value(resource);
        float *u_mat44f = material_blob::mat44f_value(resource);

        bgfx::UniformHandle *u_handler = material_blob_uniform_bgfx(resource);

        // TODO: refactor: one loop
        uint32_t offset = 0;
        for (int i = 0; i < resource->texture_count; ++i) {
            auto texture = texture::texture_get(u_texture[i]);
            bgfx::setTexture(i, u_handler[offset + i], texture, 0);
        }
        offset += resource->texture_count;

        for (int i = 0; i < resource->vec4f_count; ++i) {
            bgfx::setUniform(u_handler[offset + i], &u_vec4f[4*i], 1);
        }
        offset += resource->vec4f_count;

        for (int i = 0; i < resource->mat33f_count; ++i) {
            bgfx::setUniform(u_handler[offset + i], &u_mat33f[9*i], 1);
        }
        offset += resource->mat33f_count;

        for (int i = 0; i < resource->mat44f_count; ++i) {
            bgfx::setUniform(u_handler[offset + i], &u_mat44f[16*i], 1);
        }
        offset += resource->mat44f_count;


        uint64_t state = (0
                          | BGFX_STATE_RGB_WRITE
                          | BGFX_STATE_ALPHA_WRITE
                          | BGFX_STATE_DEPTH_TEST_LESS
                          | BGFX_STATE_DEPTH_WRITE
                          | BGFX_STATE_CULL_CW
                          | BGFX_STATE_MSAA
        );

        bgfx::setState(state, 0);
    }

    void submit(ct_material material) {
        uint32_t idx = map::get(_G.instace_map, material.idx,
                                UINT32_MAX);
        CETECH_ASSERT(LOG_WHERE, idx != UINT32_MAX);

        auto resource = material_blob::get(&_get_resorce(idx));
        auto shader = shader::shader_get(material_blob::shader_name(resource));
        bgfx::submit(0, shader, 0, 0);
    }

}