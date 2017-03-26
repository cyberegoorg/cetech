//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <celib/string/stringid.h>
#include "celib/containers/map.h"
#include "celib/filesystem/vio.h"
#include <engine/memory/api.h>
#include <engine/module/module.h>
#include "engine/resource/api.h"


//==============================================================================
// Structs
//==============================================================================

ARRAY_PROTOTYPE(bgfx_texture_handle_t)

MAP_PROTOTYPE(bgfx_texture_handle_t)

struct texture {
    u64 size;
};


//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct G {
    MAP_T(bgfx_texture_handle_t) handler_map;
    stringid64_t type;
} _G = {0};

static struct MemSysApiV0 MemSysApiV0;
static struct ResourceApiV0 ResourceApiV0;
static struct ApplicationApiV0 ApplicationApiV0;

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

    MemSysApiV0 = *(struct MemSysApiV0 *) module_get_engine_api(MEMORY_API_ID,
                                                                0);
    ResourceApiV0 = *(struct ResourceApiV0 *) module_get_engine_api(
            RESOURCE_API_ID, 0);
    ApplicationApiV0 = *(struct ApplicationApiV0 *) module_get_engine_api(
            APPLICATION_API_ID, 0);

    _G.type = stringid64_from_string("texture");

    MAP_INIT(bgfx_texture_handle_t, &_G.handler_map,
             MemSysApiV0.main_allocator());

    ResourceApiV0.compiler_register(_G.type, _texture_resource_compiler);

    ResourceApiV0.register_type(_G.type, texture_resource_callback);

    return 1;
}

void texture_shutdown() {
    MAP_DESTROY(bgfx_texture_handle_t, &_G.handler_map);

    _G = (struct G) {0};
}

bgfx_texture_handle_t texture_get(stringid64_t name) {
    ResourceApiV0.get(_G.type, name); // TODO: only for autoload

    return MAP_GET(bgfx_texture_handle_t, &_G.handler_map, name.id,
                   null_texture);
}