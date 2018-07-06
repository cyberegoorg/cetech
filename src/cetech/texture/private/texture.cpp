//==============================================================================
// Include
//==============================================================================

#include "corelib/allocator.h"

#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/log.h"
#include "cetech/machine/machine.h"
#include <corelib/os.h>

#include "cetech/resource/resource.h"
#include "corelib/buffer.inl"

#include <corelib/module.h>
#include <cetech/texture/texture.h>
#include <cetech/renderer/renderer.h>

#include "texture_blob.h"

int texturecompiler_init(struct ct_api_a0 *api);

//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct _G {
    uint32_t type;
    ct_alloc *allocator;
} _G;

//==============================================================================
// Compiler private
//==============================================================================


//==============================================================================
// Resource
//==============================================================================

#define TEXTURE_HANDLER_PROP CT_ID64_0("texture_handler")

void _texture_resource_online(uint64_t name,
                              struct ct_vio *input,
                              uint64_t obj) {
    const uint64_t size = input->size(input);
    char *data = CT_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

    auto resource = texture_blob::get(data);

    const ct_render_memory_t *mem = ct_renderer_a0->copy(resource + 1,
                                                         texture_blob::size(
                                                                 resource));

    ct_render_texture_handle_t texture = ct_renderer_a0->create_texture(mem,
                                                                        CT_RENDER_TEXTURE_NONE,
                                                                        0,
                                                                        NULL);

    ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(obj);
    ct_cdb_a0->set_uint64(writer, TEXTURE_HANDLER_PROP, texture.idx);
    ct_cdb_a0->write_commit(writer);
}


void _texture_resource_offline(uint64_t name,
                               uint64_t obj) {
    const uint64_t texture = ct_cdb_a0->read_uint64(obj, TEXTURE_HANDLER_PROP,
                                                    0);
    ct_renderer_a0->destroy_texture(
            (ct_render_texture_handle_t) {.idx=(uint16_t) texture});
}


static const ct_resource_type_t texture_resource_callback = {
        .online =_texture_resource_online,
        .offline =_texture_resource_offline,
};


//==============================================================================
// Interface
//==============================================================================
int texture_init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0->main_allocator(),
            .type = CT_ID32_0("texture")
    };

    texturecompiler_init(api);

    ct_resource_a0->register_type("texture", texture_resource_callback);

    return 1;
}

void texture_shutdown() {
}

ct_render_texture_handle texture_get(uint32_t name) {
    ct_resource_id rid = {.type = _G.type, .name = name};
    uint64_t obj = ct_resource_a0->get(rid);

    ct_render_texture_handle texture = {
            .idx = (uint16_t) ct_cdb_a0->read_uint64(obj, TEXTURE_HANDLER_PROP,
                                                     0)
    };

    return texture;
}

static struct ct_texture_a0 texture_api = {
        .get = texture_get
};

struct ct_texture_a0 *ct_texture_a0 = &texture_api;

static void _init_api(struct ct_api_a0 *api) {

    api->register_api("ct_texture_a0", &texture_api);
}


CETECH_MODULE_DEF(
        texture,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_os_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_renderer_a0);
        },
        {
            CT_UNUSED(reload);
            _init_api(api);
            texture_init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            texture_shutdown();
        }
)