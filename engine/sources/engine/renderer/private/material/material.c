//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <celib/string/stringid.h>
#include <engine/renderer/renderer.h>
#include "celib/filesystem/vio.h"
#include <engine/memory/memory.h>
#include <engine/module/module.h>

#include "engine/resource/resource.h"
#include "engine/renderer/private/texture/texture.h"
#include "engine/renderer/private/shader/shader.h"
#include "material_blob.h"

//==============================================================================
// Structs
//==============================================================================

ARRAY_PROTOTYPE(bgfx_program_handle_t)

ARRAY_PROTOTYPE(bgfx_uniform_handle_t)

ARRAY_PROTOTYPE(stringid64_t)

MAP_PROTOTYPE(bgfx_program_handle_t)


#define _get_resorce(idx) (_G.material_instance_data.data[_G.material_instance_offset.data[(idx)]])

#define LOG_WHERE "material"

ARRAY_PROTOTYPE(material_blob_t)


//==============================================================================
// GLobals
//==============================================================================

#define _G MaterialGlobals
struct G {
    MAP_T(u32) material_instace_map;
    ARRAY_T(u32) material_instance_offset;
    ARRAY_T(u8) material_instance_data;
    ARRAY_T(u32) material_instance_uniform_data;

    struct handlerid material_handler;
    stringid64_t type;
} _G = {0};

IMPORT_API(MemSysApi, 0);
IMPORT_API(ResourceApi, 0);

#define material_blob_uniform_bgfx(r)    ((bgfx_uniform_handle_t*) ((material_blob_vec4f_value(r)+((r)->vec4f_count))))

//==============================================================================
// Compiler private
//==============================================================================
#include "material_compiler.h"

//==============================================================================
// Resource
//==============================================================================
#include "material_resource.h"

//==============================================================================
// Interface
//==============================================================================

int material_init(get_api_fce_t get_engine_api) {
    _G = (struct G) {0};

    INIT_API(MemSysApi, MEMORY_API_ID, 0);
    INIT_API(ResourceApi, RESOURCE_API_ID, 0);

    _G.type = stringid64_from_string("material");

    handlerid_init(&_G.material_handler, MemSysApiV0.main_allocator());

    MAP_INIT(u32, &_G.material_instace_map, MemSysApiV0.main_allocator());
    ARRAY_INIT(u32, &_G.material_instance_offset, MemSysApiV0.main_allocator());
    ARRAY_INIT(u8, &_G.material_instance_data, MemSysApiV0.main_allocator());

    ResourceApiV0.compiler_register(_G.type, _material_resource_compiler);
    ResourceApiV0.register_type(_G.type, material_resource_callback);

    return 1;
}

void material_shutdown() {
    handlerid_destroy(&_G.material_handler);

    MAP_DESTROY(u32, &_G.material_instace_map);
    ARRAY_DESTROY(u32, &_G.material_instance_offset);
    ARRAY_DESTROY(u8, &_G.material_instance_data);

    _G = (struct G) {0};
}

static const material_t null_material = {0};

material_t material_create(stringid64_t name) {
    struct material_blob *resource = ResourceApiV0.get(_G.type, name);

    u32 size = sizeof(struct material_blob) +
               (resource->uniforms_count * sizeof(char) * 32) +
               (resource->texture_count * sizeof(stringid64_t)) +
               (resource->vec4f_count * sizeof(cel_vec4f_t)) +
               (resource->mat44f_count * sizeof(cel_mat44f_t)) +
               (resource->mat33f_count * sizeof(mat33f_t));

    handler_t h = handlerid_handler_create(&_G.material_handler);

    u32 idx = (u32) ARRAY_SIZE(&_G.material_instance_offset);

    MAP_SET(u32, &_G.material_instace_map, h.h, idx);

    u32 offset = ARRAY_SIZE(&_G.material_instance_data);
    ARRAY_PUSH(u8, &_G.material_instance_data, (u8 *) resource, size);
    ARRAY_PUSH_BACK(u32, &_G.material_instance_offset, offset);

    // write bgfx uniform handlers
    bgfx_uniform_handle_t bgfx_uniforms[resource->uniforms_count];
    const char *u_names = (const char *) (resource + 1);

    u32 off = 0;
    u32 tmp_off = 0;
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

    ARRAY_PUSH(u8, &_G.material_instance_data, (u8 *) bgfx_uniforms,
               sizeof(bgfx_uniform_handle_t) * resource->uniforms_count);

    return (material_t) {.h=h};
}


u32 material_get_texture_count(material_t material) {
    u32 idx = MAP_GET(u32, &_G.material_instace_map, material.idx, UINT32_MAX);

    if (idx == UINT32_MAX) {
        return 0;
    }

    struct material_blob *resource = (struct material_blob *) &_get_resorce(
            idx);

    return resource->texture_count;
}

u32 _material_find_slot(struct material_blob *resource,
                        const char *name) {
    const char *u_names = (const char *) (resource + 1);
    for (u32 i = 0; i < resource->uniforms_count; ++i) {
        if (cel_strcmp(&u_names[i * 32], name) != 0) {
            continue;
        }

        return i;
    }

    return UINT32_MAX;
}

void material_set_texture(material_t material,
                          const char *slot,
                          stringid64_t texture) {

    u32 idx = MAP_GET(u32, &_G.material_instace_map, material.idx, UINT32_MAX);

    if (idx == UINT32_MAX) {
        return;
    }

    struct material_blob *resource = (struct material_blob *) &_get_resorce(idx);


    stringid64_t *u_texture = material_blob_texture_names(resource);

    int slot_idx = _material_find_slot(resource, slot);

    u_texture[slot_idx] = texture;
}

void material_set_vec4f(material_t material,
                        const char *slot,
                        cel_vec4f_t v) {

    u32 idx = MAP_GET(u32, &_G.material_instace_map, material.idx, UINT32_MAX);

    if (idx == UINT32_MAX) {
        return;
    }

    struct material_blob *resource = (struct material_blob *) &_get_resorce(
            idx);

    cel_vec4f_t *u_vec4f = material_blob_vec4f_value(resource);

    int slot_idx = _material_find_slot(resource, slot);

    u_vec4f[slot_idx - (resource->texture_count)] = v;
}

void material_set_mat33f(material_t material,
                         const char *slot,
                         mat33f_t v) {

    u32 idx = MAP_GET(u32, &_G.material_instace_map, material.idx, UINT32_MAX);

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

void material_set_mat44f(material_t material,
                         const char *slot,
                         cel_mat44f_t v) {
    u32 idx = MAP_GET(u32, &_G.material_instace_map, material.idx, UINT32_MAX);

    if (idx == UINT32_MAX) {
        return;
    }

    struct material_blob *resource = (struct material_blob *) &_get_resorce(
            idx);

    cel_mat44f_t *u_mat44f = material_blob_mat44f_value(resource);

    int slot_idx = _material_find_slot(resource, slot);

    u_mat44f[slot_idx - (resource->texture_count + resource->vec4f_count +
                         resource->mat33f_count)] = v;
}


void material_use(material_t material) {
    u32 idx = MAP_GET(u32, &_G.material_instace_map, material.idx, UINT32_MAX);

    if (idx == UINT32_MAX) {
        return;
    }

    struct material_blob *resource = (struct material_blob *) &_get_resorce(
            idx);

    stringid64_t *u_texture = material_blob_texture_names(resource);
    cel_vec4f_t *u_vec4f = material_blob_vec4f_value(resource);
    mat33f_t *u_mat33f = material_blob_mat33f_value(resource);
    cel_mat44f_t *u_mat44f = material_blob_mat44f_value(resource);

    bgfx_uniform_handle_t *u_handler = material_blob_uniform_bgfx(resource);


    // TODO: refactor: one loop
    u32 offset = 0;
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


    u64 state = (0
                 | BGFX_STATE_RGB_WRITE
                 | BGFX_STATE_ALPHA_WRITE
                 | BGFX_STATE_DEPTH_TEST_LESS
                 | BGFX_STATE_DEPTH_WRITE
                 | BGFX_STATE_CULL_CCW
                 | BGFX_STATE_MSAA
    );

    bgfx_set_state(state, 0);
}

void material_submit(material_t material) {
    u32 idx = MAP_GET(u32, &_G.material_instace_map, material.idx, UINT32_MAX);
    CEL_ASSERT(LOG_WHERE, idx != UINT32_MAX);

    struct material_blob *resource = (struct material_blob *) &_get_resorce(
            idx);
    bgfx_submit(0, shader_get(resource->shader_name), 0, 0);
}
