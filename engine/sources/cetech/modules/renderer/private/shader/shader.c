//==============================================================================
// Include
//==============================================================================


#include <bgfx/c99/bgfx.h>

#include "../../../../core/allocator.h"
#include "../../../../core/hash.h"
#include "../../../../core/map.inl"

#include "../../../../kernel/application.h"
#include "../../../../kernel/config.h"

#include "../../../../kernel/resource.h"
#include "../../../../core/memory.h"
#include "../../../../core/module.h"


//==============================================================================
// Structs
//==============================================================================

ARRAY_PROTOTYPE(bgfx_program_handle_t)

MAP_PROTOTYPE(bgfx_program_handle_t)

struct shader {
    uint64_t vs_size;
    uint64_t fs_size;
    // uint8_t vs [vs_size]
    // uint8_t fs [fs_size]
};


//==============================================================================
// GLobals
//==============================================================================

#define _G ShaderResourceGlobals
struct G {
    MAP_T(bgfx_program_handle_t) handler_map;
    stringid64_t type;
} _G = {0};


IMPORT_API(memory_api_v0)
IMPORT_API(resource_api_v0)
IMPORT_API(app_api_v0)

//==============================================================================
// Compiler private
//==============================================================================
#ifdef CETECH_CAN_COMPILE
#include "shader_compiler.h"
#endif

//==============================================================================
// Resource
//==============================================================================
#include "shader_resource.h"
#include "../../../../kernel/private/module.h"

//==============================================================================
// Interface
//==============================================================================

int shader_init() {
    _G = (struct G) {0};

    memory_api_v0 = *(struct memory_api_v0 *) module_get_engine_api(MEMORY_API_ID);
    resource_api_v0 = *(struct resource_api_v0 *) module_get_engine_api(RESOURCE_API_ID);
    app_api_v0 = *(struct app_api_v0 *) module_get_engine_api(
            APPLICATION_API_ID);

    _G.type = stringid64_from_string("shader");

    MAP_INIT(bgfx_program_handle_t, &_G.handler_map,
             memory_api_v0.main_allocator());

    resource_api_v0.register_type(_G.type, shader_resource_callback);
#ifdef CETECH_CAN_COMPILE
    resource_api_v0.compiler_register(_G.type, _shader_resource_compiler);
#endif
    return 1;
}

void shader_shutdown() {
    MAP_DESTROY(bgfx_program_handle_t, &_G.handler_map);
    _G = (struct G) {0};
}

bgfx_program_handle_t shader_get(stringid64_t name) {
    struct shader *resource = resource_api_v0.get(_G.type, name);
    return MAP_GET(bgfx_program_handle_t, &_G.handler_map, name.id,
                   null_program);
}