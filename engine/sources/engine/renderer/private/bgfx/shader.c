//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <celib/string/stringid.h>
#include "celib/containers/map.h"
#include "celib/os/process.h"
#include "celib/yaml/yaml.h"
#include "celib/filesystem/path.h"
#include "celib/filesystem/vio.h"
#include "celib/filesystem/filesystem.h"

#include "engine/application/application.h"
#include "engine/resource/types.h"
#include <engine/memory/memsys.h>
#include <engine/plugin/plugin.h>


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

void *shader_resource_loader(struct vio *input,
                             struct cel_allocator *allocator) {
    const i64 size = cel_vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    cel_vio_read(input, data, 1, size);

    return data;
}

void shader_resource_unloader(void *new_data,
                              struct cel_allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
}

void shader_resource_online(stringid64_t name,
                            void *data) {
    struct shader *resource = data;

    const bgfx_memory_t *vs_mem = bgfx_alloc(resource->vs_size);
    const bgfx_memory_t *fs_mem = bgfx_alloc(resource->fs_size);

    memory_copy(vs_mem->data, (resource + 1), resource->vs_size);
    memory_copy(fs_mem->data, ((char *) (resource + 1)) + resource->vs_size,
                resource->fs_size);

    bgfx_shader_handle_t vs_shader = bgfx_create_shader(vs_mem);
    bgfx_shader_handle_t fs_shader = bgfx_create_shader(fs_mem);

    bgfx_program_handle_t program = bgfx_create_program(vs_shader, fs_shader,
                                                        1);

    MAP_SET(bgfx_program_handle_t, &_G.handler_map, name.id, program);
}

static const bgfx_program_handle_t null_program = {0};

void shader_resource_offline(stringid64_t name,
                             void *data) {

    bgfx_program_handle_t program = MAP_GET(bgfx_program_handle_t,
                                            &_G.handler_map, name.id,
                                            null_program);

    if (program.idx == null_program.idx) {
        return;
    }

    bgfx_destroy_program(program);

    MAP_REMOVE(bgfx_program_handle_t, &_G.handler_map, name.id);
}

void *shader_resource_reloader(stringid64_t name,
                               void *old_data,
                               void *new_data,
                               struct cel_allocator *allocator) {
    shader_resource_offline(name, old_data);
    shader_resource_online(name, new_data);

    CEL_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t shader_resource_callback = {
        .loader = shader_resource_loader,
        .unloader =shader_resource_unloader,
        .online =shader_resource_online,
        .offline =shader_resource_offline,
        .reloader = shader_resource_reloader
};


//==============================================================================
// Interface
//==============================================================================

int shader_resource_init() {
    _G = (struct G) {0};

    MemSysApiV0 = *(struct MemSysApiV0 *) plugin_get_engine_api(MEMORY_API_ID,
                                                                0);
    ResourceApiV0 = *(struct ResourceApiV0 *) plugin_get_engine_api(
            RESOURCE_API_ID, 0);
    ApplicationApiV0 = *(struct ApplicationApiV0 *) plugin_get_engine_api(
            APPLICATION_API_ID, 0);

    _G.type = stringid64_from_string("shader");

    MAP_INIT(bgfx_program_handle_t, &_G.handler_map,
             MemSysApiV0.main_allocator());

    ResourceApiV0.compiler_register(_G.type, _shader_resource_compiler);
    ResourceApiV0.register_type(_G.type, shader_resource_callback);

    return 1;
}

void shader_resource_shutdown() {
    MAP_DESTROY(bgfx_program_handle_t, &_G.handler_map);
    _G = (struct G) {0};
}

bgfx_program_handle_t shader_resource_get(stringid64_t name) {
    struct shader *resource = ResourceApiV0.get(_G.type, name);
    return MAP_GET(bgfx_program_handle_t, &_G.handler_map, name.id,
                   null_program);
}