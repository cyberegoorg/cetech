//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <cetech/allocator.h>
#include <cetech/stringid.h>
#include <cetech/map.inl>
#include <cetech/vio.h>
#include <cetech/memory.h>
#include <cetech/application/module.h>
#include <cetech/resource.h>


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
#include "texture_compiler.h"

//==============================================================================
// Resource
//==============================================================================
#include "texture_resource.h"

//==============================================================================
// Interface
//==============================================================================

int texture_init() {
    _G = (struct G) {0};

    memory_api_v0 = *(struct memory_api_v0 *) module_get_engine_api(MEMORY_API_ID);
    resource_api_v0 = *(struct resource_api_v0 *) module_get_engine_api(
            RESOURCE_API_ID);
    app_api_v0 = *(struct app_api_v0 *) module_get_engine_api(
            APPLICATION_API_ID);

    _G.type = stringid64_from_string("texture");

    MAP_INIT(bgfx_texture_handle_t, &_G.handler_map,
             memory_api_v0.main_allocator());

    resource_api_v0.compiler_register(_G.type, _texture_resource_compiler);

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