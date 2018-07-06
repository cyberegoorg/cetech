//==============================================================================
// Include
//==============================================================================

#include "corelib/allocator.h"

#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"


#include "cetech/resource/resource.h"

#include <cetech/renderer/renderer.h>
#include <cetech/material/material.h>
#include <cetech/shader/shader.h>

#include <cetech/texture/texture.h>
#include <corelib/module.h>

#include <corelib/os.h>
#include <corelib/macros.h>
#include <cetech/asset_property/asset_property.h>
#include <cetech/debugui/debugui.h>
#include <cstdio>
#include <cstring>
#include <cetech/ecs/ecs.h>
#include <cetech/mesh_renderer/mesh_renderer.h>
#include <cetech/asset_preview/asset_preview.h>


#include "material.h"

#include "material_blob.h"

int materialcompiler_init(struct ct_api_a0 *api);

//==============================================================================
// Defines
//==============================================================================

#define _G MaterialGlobals
#define LOG_WHERE "material"


//==============================================================================
// GLobals
//==============================================================================

static struct _G {
    ct_cdb_t db;
    uint32_t type;
    ct_alloc *allocator;
} _G;


//==============================================================================
// Resource
//==============================================================================

static ct_render_uniform_type_t _type_to_bgfx[] = {
        [MAT_VAR_NONE] = CT_RENDER_UNIFORM_TYPE_COUNT,
        [MAT_VAR_INT] = CT_RENDER_UNIFORM_TYPE_INT1,
        [MAT_VAR_TEXTURE] = CT_RENDER_UNIFORM_TYPE_INT1,
        [MAT_VAR_TEXTURE_HANDLER] = CT_RENDER_UNIFORM_TYPE_INT1,
        [MAT_VAR_VEC4] = CT_RENDER_UNIFORM_TYPE_VEC4,
        [MAT_VAR_MAT44] = CT_RENDER_UNIFORM_TYPE_MAT4,
};

static void online(uint64_t name,
                   struct ct_vio *input,
                   uint64_t obj) {
    CT_UNUSED(name);

    const uint64_t size = input->size(input);
    char *data = CT_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

    const material_blob::blob_t *resource = material_blob::get(data);

    auto *shader_names = material_blob::shader_name(resource);
    auto *layer_names = material_blob::layer_names(resource);
    auto *layer_offset = material_blob::layer_offset(resource);
    auto *uniforms = material_blob::uniforms(resource);
    auto *uniforms_names = material_blob::uniform_names(resource);
    auto *uniform_cout = material_blob::uniform_count(resource);
    auto *render_state = material_blob::render_state(resource);

    ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(obj);

    ct_cdb_a0->set_str(writer, CT_ID64_0("asset_name"), resource->asset_name);

    for (uint32_t i = 0; i < material_blob::layer_count(resource); ++i) {
        uint64_t layer_name = layer_names[i];
        uint64_t rstate = render_state[i];
        uint64_t shader = shader_names[i];

        uint64_t layer_object = ct_cdb_a0->create_object(_G.db, 0);
        ct_cdb_obj_o *layer_writer;
        layer_writer = ct_cdb_a0->write_begin(layer_object);

        struct ct_resource_id rid = (struct ct_resource_id) {
                .type = CT_ID32_0("shader"),
                .name = static_cast<uint32_t>(shader),
        };

        ct_cdb_a0->set_ref(layer_writer,
                           MATERIAL_SHADER_PROP, ct_resource_a0->get(rid));

        ct_cdb_a0->set_uint64(layer_writer, MATERIAL_STATE_PROP, rstate);

        uint64_t vars_obj = ct_cdb_a0->create_object(_G.db, 0);
        ct_cdb_obj_o *vars_writer = ct_cdb_a0->write_begin(vars_obj);
        ct_cdb_a0->set_ref(layer_writer, MATERIAL_VARIABLES_PROP, vars_obj);

        uint64_t uniform_offset = layer_offset[i];
        for (int j = 0; j < uniform_cout[i]; ++j) {
            auto &uniform = uniforms[uniform_offset + j];
            auto type = uniform.type;
            const char *uname = &uniforms_names[j * 32];
            uint64_t name_id = CT_ID64_0(uname);

            ct_render_uniform_handle_t handler = ct_renderer_a0->create_uniform(
                    uname,
                    _type_to_bgfx[type],
                    1);

            uint64_t var_obj = ct_cdb_a0->create_object(_G.db, 0);
            ct_cdb_obj_o *var_writer = ct_cdb_a0->write_begin(var_obj);

            ct_cdb_a0->set_uint64(var_writer, MATERIAL_VAR_HANDLER_PROP,
                                  handler.idx);

            ct_cdb_a0->set_uint64(var_writer, MATERIAL_VAR_TYPE_PROP, type);
            ct_cdb_a0->set_str(var_writer, MATERIAL_VAR_NAME_PROP, uname);

            switch (type) {
                case MAT_VAR_NONE:
                    break;

                case MAT_VAR_INT:
                    ct_cdb_a0->set_uint64(var_writer, MATERIAL_VAR_VALUE_PROP,
                                          uniform.i);
                    break;

                case MAT_VAR_TEXTURE:
                    ct_cdb_a0->set_uint64(var_writer, MATERIAL_VAR_VALUE_PROP,
                                          uniform.t);
                    break;

                case MAT_VAR_TEXTURE_HANDLER:
                    ct_cdb_a0->set_uint64(var_writer, MATERIAL_VAR_VALUE_PROP,
                                          uniform.th);
                    break;

                case MAT_VAR_COLOR4:
                case MAT_VAR_VEC4:
                    ct_cdb_a0->set_vec4(var_writer, MATERIAL_VAR_VALUE_PROP,
                                        uniform.v4);
                    break;

                case MAT_VAR_MAT44:
                    break;
            }
            ct_cdb_a0->write_commit(var_writer);

            ct_cdb_a0->set_ref(vars_writer, name_id, var_obj);
        }

        ct_cdb_a0->write_commit(vars_writer);
        ct_cdb_a0->write_commit(layer_writer);

        ct_cdb_a0->set_ref(writer, layer_name, layer_object);
    }

    ct_cdb_a0->write_commit(writer);
}

static void offline(uint64_t name,
                    uint64_t obj) {
    CT_UNUSED(name, obj);
}


static const ct_resource_type_t callback = {
        .online = online,
        .offline = offline,
        .compilator = material_compiler,
        .get_interface = get_interface
};


//==============================================================================
// Interface
//==============================================================================

static uint64_t create(uint32_t name) {
    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = _G.type,
            .name = name,
    };

    auto object = ct_resource_a0->get(rid);
    return object;
}

static void set_texture_handler(uint64_t material,
                                uint64_t layer,
                                const char *slot,
                                ct_render_texture_handle texture) {
    uint64_t layer_obj = ct_cdb_a0->read_ref(material, layer, 0);
    uint64_t variables = ct_cdb_a0->read_ref(layer_obj,
                                             MATERIAL_VARIABLES_PROP,
                                             0);
    uint64_t var = ct_cdb_a0->read_ref(variables, CT_ID64_0(slot), 0);
    ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(var);
    ct_cdb_a0->set_uint64(writer, MATERIAL_VAR_VALUE_PROP, texture.idx);
    ct_cdb_a0->set_uint64(writer, MATERIAL_VAR_TYPE_PROP,
                          MAT_VAR_TEXTURE_HANDLER);
    ct_cdb_a0->write_commit(writer);
}

static void set_texture(uint64_t material,
                        uint64_t layer,
                        const char *slot,
                        uint64_t texture) {
    uint64_t layer_obj = ct_cdb_a0->read_ref(material, layer, 0);
    uint64_t variables = ct_cdb_a0->read_ref(layer_obj,
                                             MATERIAL_VARIABLES_PROP,
                                             0);

    uint64_t var = ct_cdb_a0->read_ref(variables, CT_ID64_0(slot), 0);

    ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(var);
    ct_cdb_a0->set_uint64(writer, MATERIAL_VAR_VALUE_PROP, texture);
    ct_cdb_a0->set_uint64(writer, MATERIAL_VAR_TYPE_PROP, MAT_VAR_TEXTURE);
    ct_cdb_a0->write_commit(writer);
}


static void set_mat44f(uint64_t material,
                       uint64_t layer,
                       const char *slot,
                       float *value) {
}

static void submit(uint64_t material,
                   uint64_t _layer,
                   uint8_t viewid) {
    uint64_t layer = ct_cdb_a0->read_ref(material, _layer, 0);
    if (!layer) {
        return;
    }

    uint64_t variables = ct_cdb_a0->read_ref(layer,
                                             MATERIAL_VARIABLES_PROP,
                                             0);

    uint64_t key_count = ct_cdb_a0->prop_count(variables);
    uint64_t keys[key_count];
    ct_cdb_a0->prop_keys(variables, keys);

    uint8_t texture_stage = 0;

    for (int j = 0; j < key_count; ++j) {
        uint64_t var = ct_cdb_a0->read_ref(variables, keys[j], 0);
        uint64_t type = ct_cdb_a0->read_uint64(var, MATERIAL_VAR_TYPE_PROP, 0);

        ct_render_uniform_handle_t handle = {
                .idx = (uint16_t) ct_cdb_a0->read_uint64(var,
                                                         MATERIAL_VAR_HANDLER_PROP,
                                                         0)
        };

        switch (type) {
            case MAT_VAR_NONE:
                break;

            case MAT_VAR_INT: {
                uint32_t v = ct_cdb_a0->read_uint64(var,
                                                    MATERIAL_VAR_VALUE_PROP, 0);
                ct_renderer_a0->set_uniform(handle, &v, 1);
            }
                break;

            case MAT_VAR_TEXTURE: {
                uint64_t t = ct_cdb_a0->read_uint64(var,
                                                    MATERIAL_VAR_VALUE_PROP, 0);
                auto texture = ct_texture_a0->get(t);
                ct_renderer_a0->set_texture(texture_stage++, handle,
                                            {texture.idx}, 0);
            }
                break;

            case MAT_VAR_TEXTURE_HANDLER: {
                uint64_t t = ct_cdb_a0->read_uint64(var,
                                                    MATERIAL_VAR_VALUE_PROP, 0);
                ct_renderer_a0->set_texture(texture_stage++, handle,
                                            {.idx=(uint16_t) t}, 0);
            }
                break;

            case MAT_VAR_COLOR4:
            case MAT_VAR_VEC4: {
                float v[4] = {1.0f, 1.0f, 1.0f, 1.0f};
                ct_cdb_a0->read_vec4(var, MATERIAL_VAR_VALUE_PROP, v),
                        ct_renderer_a0->set_uniform(handle, &v, 1);
            }
                break;

            case MAT_VAR_MAT44:
                break;
        }
    }

    uint64_t shader_obj = ct_cdb_a0->read_ref(layer,
                                              MATERIAL_SHADER_PROP,
                                              0);
    auto shader = ct_shader_a0->get(shader_obj);
    uint64_t state = ct_cdb_a0->read_uint64(layer, MATERIAL_STATE_PROP, 0);

    ct_renderer_a0->set_state(state, 0);
    ct_renderer_a0->submit(viewid, {shader.idx}, 0, false);
}

static struct ct_material_a0 material_api = {
        .resource_create = create,
        .set_texture = set_texture,
        .set_texture_handler = set_texture_handler,
        .set_mat44f = set_mat44f,
        .submit = submit
};

struct ct_material_a0 *ct_material_a0 = &material_api;

static int init(struct ct_api_a0 *api) {
    _G = {
            .allocator = ct_memory_a0->system,
            .type = CT_ID32_0("material"),
            .db = ct_cdb_a0->global_db()
    };
    api->register_api("ct_material_a0", &material_api);

    ct_resource_a0->register_type("material", callback);

    materialcompiler_init(api);

    return 1;
}

static void shutdown() {
    ct_cdb_a0->destroy_db(_G.db);
}

CETECH_MODULE_DEF(
        material,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_os_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_texture_a0);
            CETECH_GET_API(api, ct_shader_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_renderer_a0);
        },
        {
            CT_UNUSED(reload);
            init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            shutdown();
        }
)