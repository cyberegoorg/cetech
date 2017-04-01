//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <celib/string/stringid.h>
#include "celib/containers/map.h"
#include "celib/filesystem/vio.h"

#include <cetech/resource/resource.h>
#include <cetech/memory/memory.h>
#include <cetech/application/private/module.h>


//==============================================================================
// Structs
//==============================================================================

ARRAY_PROTOTYPE(bgfx_program_handle_t)

MAP_PROTOTYPE(bgfx_program_handle_t)

struct shader {
    u64 vs_size;
    u64 fs_size;
    // u8 vs [vs_size]
    // u8 fs [fs_size]
};


//==============================================================================
// GLobals
//==============================================================================

#define _G ShaderResourceGlobals
struct G {
    MAP_T(bgfx_program_handle_t) handler_map;
    stringid64_t type;
} _G = {0};

static struct MemSysApiV0 MemSysApiV0;
static struct ResourceApiV0 ResourceApiV0;
static struct ApplicationApiV0 ApplicationApiV0;

//==============================================================================
// Compiler private
//==============================================================================
#include "shader_compiler.h"

//==============================================================================
// Resource
//==============================================================================
#include "shader_resource.h"

//==============================================================================
// Interface
//==============================================================================

int shader_init() {
    _G = (struct G) {0};

    MemSysApiV0 = *(struct MemSysApiV0 *) module_get_engine_api(MEMORY_API_ID,
                                                                0);
    ResourceApiV0 = *(struct ResourceApiV0 *) module_get_engine_api(
            RESOURCE_API_ID, 0);
    ApplicationApiV0 = *(struct ApplicationApiV0 *) module_get_engine_api(
            APPLICATION_API_ID, 0);

    _G.type = stringid64_from_string("shader");

    MAP_INIT(bgfx_program_handle_t, &_G.handler_map,
             MemSysApiV0.main_allocator());

    ResourceApiV0.compiler_register(_G.type, _shader_resource_compiler);
    ResourceApiV0.register_type(_G.type, shader_resource_callback);

    return 1;
}

void shader_shutdown() {
    MAP_DESTROY(bgfx_program_handle_t, &_G.handler_map);
    _G = (struct G) {0};
}

bgfx_program_handle_t shader_get(stringid64_t name) {
    struct shader *resource = ResourceApiV0.get(_G.type, name);
    return MAP_GET(bgfx_program_handle_t, &_G.handler_map, name.id,
                   null_program);
}