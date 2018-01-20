//==============================================================================
// Include
//==============================================================================

#include "cetech/core/memory/allocator.h"
#include "cetech/core/containers/map.inl"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/log/log.h"
#include "cetech/engine/machine/machine.h"
#include "cetech/core/os/process.h"
#include "cetech/core/os/vio.h"

#include "cetech/engine/resource/resource.h"
#include "cetech/core/containers/buffer.h"

#include <bgfx/bgfx.h>
#include <cetech/core/module/module.h>
#include <cetech/engine/renderer/texture.h>

#include "texture_blob.h"
#include "cetech/core/os/path.h"

using namespace celib;

int texturecompiler_init(ct_api_a0 *api);

//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct TextureResourceGlobals {
    Map<ct_texture> handler_map;
    uint64_t type;
} TextureResourceGlobals;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_process_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);

//==============================================================================
// Compiler private
//==============================================================================


//==============================================================================
// Resource
//==============================================================================

static const ct_texture null_texture = {};


void *_texture_resource_loader(ct_vio *input,
                               ct_alloc *allocator) {

    const int64_t size = input->size(input);
    char *data = CT_ALLOC(allocator, char, size);
    input->read(input, data, 1, size);

    return data;
}

void _texture_resource_unloader(void *new_data,
                                ct_alloc *allocator) {
    CT_FREE(allocator, new_data);
}

void _texture_resource_online(uint64_t name,
                              void *data) {
    auto resource = texture_blob::get(data);

    const bgfx::Memory *mem = bgfx::copy((resource + 1),
                                         texture_blob::size(resource));
    auto texture = bgfx::createTexture(mem, BGFX_TEXTURE_NONE, 0, NULL);

    map::set(_G.handler_map, name, {texture.idx});
}


void _texture_resource_offline(uint64_t name,
                               void *data) {
    CT_UNUSED(data);

    auto texture = map::get(_G.handler_map, name, null_texture);

    if (texture.idx == null_texture.idx) {
        return;
    }

    bgfx::destroy((bgfx::TextureHandle) {texture.idx});
}

void *_texture_resource_reloader(uint64_t name,
                                 void *old_data,
                                 void *new_data,
                                 ct_alloc *allocator) {
    _texture_resource_offline(name, old_data);
    _texture_resource_online(name, new_data);

    CT_FREE(allocator, old_data);

    return new_data;
}

static const ct_resource_callbacks_t texture_resource_callback = {
        .loader = _texture_resource_loader,
        .unloader =_texture_resource_unloader,
        .online =_texture_resource_online,
        .offline =_texture_resource_offline,
        .reloader = _texture_resource_reloader
};


//==============================================================================
// Interface
//==============================================================================
int texture_init(ct_api_a0 *api) {
    _G = {};

    _G.type = CT_ID64_0("texture");

    _G.handler_map.init(ct_memory_a0.main_allocator());


    texturecompiler_init(api);

    ct_resource_a0.register_type(_G.type,
                                 texture_resource_callback);

    return 1;
}

void texture_shutdown() {
    _G.handler_map.destroy();
}

ct_texture texture_get(uint64_t name) {
    ct_resource_a0.get(_G.type, name); // TODO: only for autoload

    return map::get(_G.handler_map, name, null_texture);
}

static ct_texture_a0 texture_api = {
        .get = texture_get
};

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_texture_a0", &texture_api);
}


CETECH_MODULE_DEF(
        texture,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_process_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hash_a0);
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