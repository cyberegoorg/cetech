//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <cetech/core/allocator.h>
#include <cetech/core/hash.h>
#include <cetech/core/map.inl>
#include <cetech/core/memory.h>
#include <cetech/kernel/application.h>

#include <cetech/core/module.h>
#include <cetech/kernel/resource.h>
#include <cetech/core/api.h>


//==============================================================================
// Structs
//==============================================================================

ARRAY_PROTOTYPE(bgfx_texture_handle_t)

MAP_PROTOTYPE(bgfx_texture_handle_t)

struct texture {
    uint64_t size;
};


//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct G {
    MAP_T(bgfx_texture_handle_t) handler_map;
    stringid64_t type;
} _G = {0};


IMPORT_API(memory_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(app_api_v0);

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

    USE_API(api, memory_api_v0 );
    USE_API(api, resource_api_v0 );
    USE_API(api, app_api_v0 );

    _G.type = stringid64_from_string("texture");

    MAP_INIT(bgfx_texture_handle_t, &_G.handler_map,
             memory_api_v0.main_allocator());

#ifdef CETECH_CAN_COMPILE
    resource_api_v0.compiler_register(_G.type, _texture_resource_compiler);
#endif

    resource_api_v0.register_type(_G.type, texture_resource_callback);

    return 1;
}

void texture_shutdown() {
    MAP_DESTROY(bgfx_texture_handle_t, &_G.handler_map);

    _G = (struct G) {0};
}

bgfx_texture_handle_t texture_get(stringid64_t name) {
    resource_api_v0.get(_G.type, name); // TODO: only for autoload

    return MAP_GET(bgfx_texture_handle_t, &_G.handler_map, name.id,
                   null_texture);
}