//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <cetech/celib/allocator.h>
#include <cetech/celib/map.inl>

#include <cetech/core/hash.h>
#include <cetech/core/memory.h>
#include <cetech/core/application.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>
#include <cetech/core/log.h>
#include <cetech/core/path.h>
#include <cetech/core/vio.h>

#include <cetech/modules/resource.h>
#include <cetech/core/process.h>

using namespace cetech;

//==============================================================================
// Structs
//==============================================================================

struct texture {
    uint64_t size;
};


//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct G {
    Map<bgfx_texture_handle_t> handler_map;
    uint64_t type;
} _G = {0};


IMPORT_API(memory_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(app_api_v0);
IMPORT_API(path_v0);
IMPORT_API(vio_api_v0);
IMPORT_API(process_api_v0);
IMPORT_API(log_api_v0);
IMPORT_API(hash_api_v0);

//==============================================================================
// Compiler private
//==============================================================================
#ifdef CETECH_CAN_COMPILE

#include "texture_compiler.h"

#endif

//==============================================================================
// Resource
//==============================================================================
#include "texture_resource.h"

//==============================================================================
// Interface
//==============================================================================

int texture_init(struct api_v0 *api) {
    _G = (struct G) {0};

    GET_API(api, memory_api_v0);
    GET_API(api, resource_api_v0);
    GET_API(api, app_api_v0);
    GET_API(api, path_v0);
    GET_API(api, vio_api_v0);
    GET_API(api, process_api_v0);
    GET_API(api, log_api_v0);
    GET_API(api, hash_api_v0);

    _G.type = hash_api_v0.id64_from_str("texture");

    _G.handler_map.init(memory_api_v0.main_allocator());

#ifdef CETECH_CAN_COMPILE
    resource_api_v0.compiler_register(_G.type, _texture_resource_compiler);
#endif

    resource_api_v0.register_type(_G.type, texture_resource_callback);

    return 1;
}

void texture_shutdown() {
    _G = {0};
}

bgfx_texture_handle_t texture_get(uint64_t name) {
    resource_api_v0.get(_G.type, name); // TODO: only for autoload

    return map::get(_G.handler_map, name, null_texture);
}