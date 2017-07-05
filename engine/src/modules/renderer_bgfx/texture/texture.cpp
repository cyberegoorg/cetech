//==============================================================================
// Include
//==============================================================================

#include <cetech/celib/allocator.h>
#include <cetech/celib/map.inl>

#include <cetech/kernel/hash.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/application.h>
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
    int init(api_v0 *api);
}

//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct TextureResourceGlobals {
    Map<bgfx::TextureHandle> handler_map;
    uint64_t type;
} TextureResourceGlobals;


CETECH_DECL_API(memory_api_v0);
CETECH_DECL_API(resource_api_v0);
CETECH_DECL_API(app_api_v0);
CETECH_DECL_API(os_path_v0);
CETECH_DECL_API(os_vio_api_v0);
CETECH_DECL_API(os_process_api_v0);
CETECH_DECL_API(log_api_v0);
CETECH_DECL_API(hash_api_v0);

//==============================================================================
// Compiler private
//==============================================================================


//==============================================================================
// Resource
//==============================================================================

namespace texture_resource {

    static const bgfx::TextureHandle null_texture = {0};


    void *_texture_resource_loader(struct os_vio *input,
                                   struct allocator *allocator) {
        const int64_t size = os_vio_api_v0.size(input);
        char *data = CETECH_ALLOCATE(allocator, char, size);
        os_vio_api_v0.read(input, data, 1, size);

        return data;
    }

    void _texture_resource_unloader(void *new_data,
                                    struct allocator *allocator) {
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
                                     struct allocator *allocator) {
        _texture_resource_offline(name, old_data);
        _texture_resource_online(name, new_data);

        CETECH_FREE(allocator, old_data);

        return new_data;
    }

    static const resource_callbacks_t texture_resource_callback = {
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
    int texture_init(struct api_v0 *api) {

        CETECH_GET_API(api, memory_api_v0);
        CETECH_GET_API(api, resource_api_v0);
        CETECH_GET_API(api, app_api_v0);
        CETECH_GET_API(api, os_path_v0);
        CETECH_GET_API(api, os_vio_api_v0);
        CETECH_GET_API(api, os_process_api_v0);
        CETECH_GET_API(api, log_api_v0);
        CETECH_GET_API(api, hash_api_v0);

        _G = {0};

        _G.type = hash_api_v0.id64_from_str("texture");

        _G.handler_map.init(memory_api_v0.main_allocator());

#ifdef CETECH_CAN_COMPILE
        texture_compiler::init(api);
#endif

        resource_api_v0.register_type(_G.type,
                                      texture_resource::texture_resource_callback);

        return 1;
    }

    void texture_shutdown() {
        _G.handler_map.destroy();
    }

    bgfx::TextureHandle texture_get(uint64_t name) {
        resource_api_v0.get(_G.type, name); // TODO: only for autoload

        return map::get(_G.handler_map, name, texture_resource::null_texture);
    }

}