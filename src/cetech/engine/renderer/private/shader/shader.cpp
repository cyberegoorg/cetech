//==============================================================================
// Include
//==============================================================================

#include <cstdio>

#include "cetech/core/memory/allocator.h"
#include "cetech/core/containers/map.inl"
#include "cetech/core/containers/buffer.h"

#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/log/log.h"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/engine/machine/machine.h"

#include "cetech/engine/resource/resource.h"

#include <bgfx/bgfx.h>
#include <cetech/core/module/module.h>
#include <cetech/engine/renderer/shader.h>

#include "shader_blob.h"
#include "cetech/core/os/path.h"
#include "cetech/core/os/process.h"
#include "cetech/core/os/vio.h"

using namespace celib;

int shadercompiler_init(ct_api_a0 *api);

//==============================================================================
// GLobals
//==============================================================================

#define _G ShaderResourceGlobals
struct _G {
    Map<ct_shader> handler_map;
    uint64_t type;
    ct_alloc* allocator;
} _G;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_process_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_cdb_a0);

//==============================================================================
// Resource
//==============================================================================

#define SHADER_PROP CT_ID64_0("shader")

static void online(uint64_t name,
                   struct ct_vio* input,
                   struct ct_cdb_object_t *obj) {
    const uint64_t size = input->size(input);
    char *data = CT_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

    auto *resource = shader_blob::get(data);

    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

    if (resource) {

        auto vs_mem = bgfx::alloc(shader_blob::vs_size(resource));
        auto fs_mem = bgfx::alloc(shader_blob::fs_size(resource));

        memcpy(vs_mem->data, (resource + 1), resource->vs_size);
        memcpy(fs_mem->data, ((char *) (resource + 1)) + resource->vs_size,
               resource->fs_size);

        auto vs_shader = bgfx::createShader(vs_mem);
        auto fs_shader = bgfx::createShader(fs_mem);
        program = bgfx::createProgram(vs_shader, fs_shader, 1);

    }

    struct ct_cdb_writer_t *writer = ct_cdb_a0.write_begin(obj);
    ct_cdb_a0.set_uint64(writer, SHADER_PROP, program.idx);
    ct_cdb_a0.write_commit(writer);
}

static void offline(uint64_t name,
                    struct ct_cdb_object_t *obj) {
    CT_UNUSED(name);

    const uint64_t program = ct_cdb_a0.read_uint64(obj, SHADER_PROP, 0);
    bgfx::destroy((bgfx::ProgramHandle) {.idx=(uint16_t)program});
}

static const ct_resource_type_t callback = {
        .online = online,
        .offline = offline,
};

//==============================================================================
// Interface
//==============================================================================
int shader_init(ct_api_a0 *api) {
    _G = {.allocator = ct_memory_a0.main_allocator()};

    _G.type = CT_ID64_0("shader");

    _G.handler_map.init(ct_memory_a0.main_allocator());

    ct_resource_a0.register_type(_G.type,
                                 callback);
    shadercompiler_init(api);

    return 1;
}

void shader_shutdown() {
    _G.handler_map.destroy();
}

ct_shader shader_get(ct_cdb_object_t* shader) {
    const uint64_t idx = ct_cdb_a0.read_uint64(shader, SHADER_PROP, 0);
    return (ct_shader){.idx=(uint16_t)idx};
}

static ct_shader_a0 shader_api = {
        .get = shader_get
};

static void _init_api(struct ct_api_a0 *api) {

    api->register_api("ct_shader_a0", &shader_api);
}

CETECH_MODULE_DEF(
        shader,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_process_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_cdb_a0);
        },
        {
            CT_UNUSED(reload);
            _init_api(api);
            shader_init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            shader_shutdown();
        }
)