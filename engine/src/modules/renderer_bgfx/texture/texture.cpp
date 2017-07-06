//==============================================================================
// Include
//==============================================================================

#include <cetech/celib/allocator.h>
#include <cetech/celib/map.inl>

#include <cetech/kernel/hash.h>
#include <cetech/kernel/memory.h>
#include <cetech/modules/application.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/os.h>


#include <cetech/modules/resource.h>
#include <cetech/celib/string_stream.h>

#include <bgfx/bgfx.h>

#include "texture_blob.h"

using namespace cetech;
using namespace string_stream;


namespace texture_compiler {
    int init(ct_api_a0 *api);
}

//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct TextureResourceGlobals {
    Map<bgfx::TextureHandle> handler_map;
    uint64_t type;
} TextureResourceGlobals;


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_app_a0);
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

namespace texture_resource {

    static const bgfx::TextureHandle null_texture = {0};


    void *_texture_resource_loader(struct ct_vio *input,
                                   struct ct_allocator *allocator) {

        const int64_t size = input->size(input->inst);
        char *data = CETECH_ALLOCATE(allocator, char, size);
        input->read(input->inst, data, 1, size);

        return data;
    }

    void _texture_resource_unloader(void *new_data,
                                    struct ct_allocator *allocator) {
        CETECH_FREE(allocator, new_data);
    }

    void _texture_resource_online(uint64_t name,
                                  void *data) {
        auto resource = texture_blob::get(data);

        const bgfx::Memory *mem = bgfx::copy((resource + 1),
                                             texture_blob::size(resource));
        auto texture = bgfx::createTexture(mem, BGFX_TEXTURE_NONE, 0, NULL);

        map::set(_G.handler_map, name, texture);
    }


    void _texture_resource_offline(uint64_t name,
                                   void *data) {

        auto texture = map::get(_G.handler_map, name, null_texture);

        if (texture.idx == null_texture.idx) {
            return;
        }

        bgfx::destroyTexture(texture);
    }

    void *_texture_resource_reloader(uint64_t name,
                                     void *old_data,
                                     void *new_data,
                                     struct ct_allocator *allocator) {
        _texture_resource_offline(name, old_data);
        _texture_resource_online(name, new_data);

        CETECH_FREE(allocator, old_data);

        return new_data;
    }

    static const ct_resource_callbacks_t texture_resource_callback = {
            .loader = _texture_resource_loader,
            .unloader =_texture_resource_unloader,
            .online =_texture_resource_online,
            .offline =_texture_resource_offline,
            .reloader = _texture_resource_reloader
    };

}

//==============================================================================
// Interface
//==============================================================================
namespace texture {
    int texture_init(struct ct_api_a0 *api) {

        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_resource_a0);
        CETECH_GET_API(api, ct_app_a0);
        CETECH_GET_API(api, ct_path_a0);
        CETECH_GET_API(api, ct_vio_a0);
        CETECH_GET_API(api, ct_process_a0);
        CETECH_GET_API(api, ct_log_a0);
        CETECH_GET_API(api, ct_hash_a0);

        _G = {0};

        _G.type = ct_hash_a0.id64_from_str("texture");

        _G.handler_map.init(ct_memory_a0.main_allocator());

#ifdef CETECH_CAN_COMPILE
        texture_compiler::init(api);
#endif

        ct_resource_a0.register_type(_G.type,
                                      texture_resource::texture_resource_callback);

        return 1;
    }

    void texture_shutdown() {
        _G.handler_map.destroy();
    }

    bgfx::TextureHandle texture_get(uint64_t name) {
        ct_resource_a0.get(_G.type, name); // TODO: only for autoload

        return map::get(_G.handler_map, name, texture_resource::null_texture);
    }

}