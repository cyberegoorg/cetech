//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <cetech/celib/allocator.h>
#include <cetech/celib/array.inl>
#include <cetech/celib/map.inl>
#include <cetech/celib/handler.inl>

#include <cetech/core/hash.h>
#include <cetech/core/memory.h>
#include <cetech/core/module.h>
#include <cetech/core/vio.h>
#include <cetech/core/api.h>
#include <cetech/core/path.h>

#include <cetech/modules/resource.h>
#include <cetech/core/errors.h>

#include "cetech/modules/renderer.h"
#include "texture.h"
#include "shader.h"

#include "material_blob.h"

IMPORT_API(memory_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(path_v0);
IMPORT_API(vio_api_v0);
IMPORT_API(hash_api_v0);


using namespace cetech;

//==============================================================================
// Structs
//==============================================================================

#define LOG_WHERE "material"

#define _get_resorce(idx) (_G.material_instance_data[_G.material_instance_offset[(idx)]])

//==============================================================================
// GLobals
//==============================================================================
namespace {
    struct MaterialGlobals {
        Map<uint32_t> material_instace_map;
        Array<uint32_t> material_instance_offset;
        Array<uint8_t> material_instance_data;
        Array<uint32_t> material_instance_uniform_data;

        Handler<uint32_t> material_handler;
        uint64_t type;
    } _G = {0};
}

#define material_blob_uniform_bgfx(r)    ((bgfx_uniform_handle_t*) ((material_blob_vec4f_value(r)+((r)->vec4f_count))))

//==============================================================================
// Compiler private
//==============================================================================
#ifdef CETECH_CAN_COMPILE

#include "material_compiler.inl"

#endif

//==============================================================================
// Resource
//==============================================================================
#include "material_resource.inl"

//==============================================================================
// Interface
//==============================================================================
namespace {
    static const resource_callbacks_t resource_callback = {
            .loader = material_resource::loader,
            .unloader = material_resource::unloader,
            .online = material_resource::online,
            .offline = material_resource::offline,
            .reloader = material_resource::reloader
    };
}

namespace material {

    int init(struct api_v0 *api) {
        GET_API(api, memory_api_v0);
        GET_API(api, resource_api_v0);
        GET_API(api, path_v0);
        GET_API(api, vio_api_v0);
        GET_API(api, hash_api_v0);

        _G = {0};

        _G.type = hash_api_v0.id64_from_str("material");

        _G.material_handler.init(memory_api_v0.main_allocator());

        _G.material_instace_map.init(memory_api_v0.main_allocator());
        _G.material_instance_offset.init(memory_api_v0.main_allocator());
        _G.material_instance_data.init(memory_api_v0.main_allocator());

        resource_api_v0.register_type(_G.type, resource_callback);
#ifdef CETECH_CAN_COMPILE
        resource_api_v0.compiler_register(_G.type,
                                          material_resource_compiler::compiler);
#endif
        return 1;
    }

    void shutdown() {
        _G = {0};
    }

    static const material_t null_material = {0};

    material_t create(uint64_t name) {
        struct material_blob *resource = (material_blob *) resource_api_v0.get(
                _G.type, name);

        uint32_t size = sizeof(struct material_blob) +
                        (resource->uniforms_count * sizeof(char) * 32) +
                        (resource->texture_count * sizeof(uint64_t)) +
                        (resource->vec4f_count * sizeof(vec4f_t)) +
                        (resource->mat44f_count * sizeof(mat44f_t)) +
                        (resource->mat33f_count * sizeof(mat33f_t));

        uint32_t h = handler::create(_G.material_handler);

        uint32_t idx = (uint32_t) array::size(_G.material_instance_offset);

        map::set(_G.material_instace_map, h, idx);

        uint32_t offset = array::size(_G.material_instance_data);
        array::push(_G.material_instance_data, (uint8_t *) resource, size);
        array::push_back(_G.material_instance_offset, offset);

        // write bgfx uniform handlers
        bgfx_uniform_handle_t bgfx_uniforms[resource->uniforms_count];
        const char *u_names = (const char *) (resource + 1);

        uint32_t off = 0;
        uint32_t tmp_off = 0;
        off += resource->texture_count;
        for (int i = 0; i < resource->texture_count; ++i) {
            bgfx_uniforms[i] = bgfx_create_uniform(&u_names[i * 32],
                                                   BGFX_UNIFORM_TYPE_INT1, 1);
        }

        tmp_off = off;
        off += resource->vec4f_count;
        for (int i = tmp_off; i < off; ++i) {
            bgfx_uniforms[i] = bgfx_create_uniform(&u_names[i * 32],
                                                   BGFX_UNIFORM_TYPE_VEC4, 1);
        }

        tmp_off = off;
        off += resource->mat33f_count;
        for (int i = tmp_off; i < off; ++i) {
            bgfx_uniforms[i] = bgfx_create_uniform(&u_names[i * 32],
                                                   BGFX_UNIFORM_TYPE_MAT3, 1);
        }

        tmp_off = off;
        off += resource->mat44f_count;
        for (int i = tmp_off; i < off; ++i) {
            bgfx_uniforms[i] = bgfx_create_uniform(&u_names[i * 32],
                                                   BGFX_UNIFORM_TYPE_MAT4, 1);
        }

        array::push(_G.material_instance_data, (uint8_t *) bgfx_uniforms,
                    sizeof(bgfx_uniform_handle_t) * resource->uniforms_count);

        return (material_t) {.idx=h};
    }


    uint32_t get_texture_count(material_t material) {
        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return 0;
        }

        struct material_blob *resource = (struct material_blob *) &_get_resorce(
                idx);

        return resource->texture_count;
    }

    uint32_t _material_find_slot(struct material_blob *resource,
                                 const char *name) {
        const char *u_names = (const char *) (resource + 1);
        for (uint32_t i = 0; i < resource->uniforms_count; ++i) {
            if (strcmp(&u_names[i * 32], name) != 0) {
                continue;
            }

            return i;
        }

        return UINT32_MAX;
    }

    void set_texture(material_t material,
                     const char *slot,
                     uint64_t texture) {

        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        struct material_blob *resource = (struct material_blob *) &_get_resorce(
                idx);


        uint64_t *u_texture = material_blob_texture_names(resource);

        int slot_idx = _material_find_slot(resource, slot);

        u_texture[slot_idx] = texture;
    }

    void set_vec4f(material_t material,
                   const char *slot,
                   vec4f_t v) {

        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        struct material_blob *resource = (struct material_blob *) &_get_resorce(
                idx);

        vec4f_t *u_vec4f = material_blob_vec4f_value(resource);

        int slot_idx = _material_find_slot(resource, slot);

        u_vec4f[slot_idx - (resource->texture_count)] = v;
    }

    void set_mat33f(material_t material,
                    const char *slot,
                    mat33f_t v) {

        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        struct material_blob *resource = (struct material_blob *) &_get_resorce(
                idx);

        mat33f_t *u_mat33f = material_blob_mat33f_value(resource);

        int slot_idx = _material_find_slot(resource, slot);

        u_mat33f[slot_idx -
                 (resource->texture_count + resource->vec4f_count)] = v;
    }

    void set_mat44f(material_t material,
                    const char *slot,
                    mat44f_t v) {
        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        struct material_blob *resource = (struct material_blob *) &_get_resorce(
                idx);

        mat44f_t *u_mat44f = material_blob_mat44f_value(resource);

        int slot_idx = _material_find_slot(resource, slot);

        u_mat44f[slot_idx - (resource->texture_count + resource->vec4f_count +
                             resource->mat33f_count)] = v;
    }


    void use(material_t material) {
        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        struct material_blob *resource = (struct material_blob *) &_get_resorce(
                idx);

        uint64_t *u_texture = material_blob_texture_names(resource);
        vec4f_t *u_vec4f = material_blob_vec4f_value(resource);
        mat33f_t *u_mat33f = material_blob_mat33f_value(resource);
        mat44f_t *u_mat44f = material_blob_mat44f_value(resource);

        bgfx_uniform_handle_t *u_handler = material_blob_uniform_bgfx(resource);


        // TODO: refactor: one loop
        uint32_t offset = 0;
        for (int i = 0; i < resource->texture_count; ++i) {
            bgfx_texture_handle_t texture = texture_get(u_texture[i]);
            bgfx_set_texture(i, u_handler[offset + i], texture, 0);
        }
        offset += resource->texture_count;


        for (int i = 0; i < resource->vec4f_count; ++i) {
            bgfx_set_uniform(u_handler[offset + i], &u_vec4f[i], 1);
        }
        offset += resource->vec4f_count;


        for (int i = 0; i < resource->mat33f_count; ++i) {
            bgfx_set_uniform(u_handler[offset + i], &u_mat33f[i], 1);
        }
        offset += resource->mat33f_count;

        for (int i = 0; i < resource->mat44f_count; ++i) {
            bgfx_set_uniform(u_handler[offset + i], &u_mat44f[i], 1);
        }
        offset += resource->mat44f_count;


        uint64_t state = (0
                          | BGFX_STATE_RGB_WRITE
                          | BGFX_STATE_ALPHA_WRITE
                          | BGFX_STATE_DEPTH_TEST_LESS
                          | BGFX_STATE_DEPTH_WRITE
                          | BGFX_STATE_CULL_CCW
                          | BGFX_STATE_MSAA
        );

        bgfx_set_state(state, 0);
    }

    void submit(material_t material) {
        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);
        CETECH_ASSERT(LOG_WHERE, idx != UINT32_MAX);

        struct material_blob *resource = (struct material_blob *) &_get_resorce(
                idx);
        bgfx_submit(0, shader_get(resource->shader_name), 0, 0);
    }

}