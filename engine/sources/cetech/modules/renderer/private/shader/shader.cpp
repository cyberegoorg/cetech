//==============================================================================
// Include
//==============================================================================


#include <bgfx/c99/bgfx.h>

#include <cetech/core/memory/allocator.h>
#include <cetech/core/container/map.inl>

#include <cetech/core/hash.h>
#include <cetech/core/application.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/os/path.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>
#include <cetech/core/os/vio.h>

#include <cetech/core/resource/resource.h>
#include <cetech/core/os/process.h>

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
    uint64_t type;
} _G = {0};


IMPORT_API(memory_api_v0)
IMPORT_API(resource_api_v0)
IMPORT_API(app_api_v0)
IMPORT_API(path_v0)
IMPORT_API(vio_api_v0)
IMPORT_API(process_api_v0)
IMPORT_API(log_api_v0)
IMPORT_API(hash_api_v0)

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

//==============================================================================
// Interface
//==============================================================================

int shader_init(struct api_v0 *api) {
    _G = (struct G) {0};

    GET_API(api, memory_api_v0);
    GET_API(api, resource_api_v0);
    GET_API(api, app_api_v0);
    GET_API(api, path_v0);
    GET_API(api, vio_api_v0);
    GET_API(api, process_api_v0);
    GET_API(api, log_api_v0);
    GET_API(api, hash_api_v0);


    _G.type = hash_api_v0.id64_from_str("shader");

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

bgfx_program_handle_t shader_get(uint64_t name) {
    struct shader *resource = (shader *) resource_api_v0.get(_G.type, name);
    return MAP_GET(bgfx_program_handle_t, &_G.handler_map, name,
                   null_program);
}