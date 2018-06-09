//==============================================================================
// Include
//==============================================================================

#include <cstdio>

#include "cetech/kernel/memory/allocator.h"
#include "cetech/kernel/containers/buffer.h"

#include "cetech/kernel/memory/memory.h"
#include "cetech/kernel/api/api_system.h"
#include "cetech/kernel/log/log.h"

#include "cetech/kernel/hashlib/hashlib.h"
#include "cetech/engine/machine/machine.h"

#include "cetech/engine/resource/resource.h"

#include <cetech/kernel/module/module.h>
#include <cetech/engine/shader/shader.h>
#include <cetech/engine/renderer/renderer.h>

#include "shader_blob.h"
#include "cetech/kernel/os/path.h"
#include "cetech/kernel/os/process.h"
#include "cetech/kernel/os/vio.h"


int shadercompiler_init(ct_api_a0 *api);

//==============================================================================
// GLobals
//==============================================================================

#define _G ShaderResourceGlobals
struct _G {
    uint64_t type;
    ct_alloc* allocator;
} _G;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_process_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_cdb_a0);

//==============================================================================
// Resource
//==============================================================================

#define SHADER_PROP CT_ID64_0("shader")

static void online(uint64_t name,
                   struct ct_vio* input,
                   struct ct_cdb_obj_t *obj) {
    const uint64_t size = input->size(input);
    char *data = CT_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

    auto *resource = shader_blob::get(data);

    ct_render_program_handle_t program;

    if (resource) {

        auto vs_mem = ct_renderer_a0.alloc(shader_blob::vs_size(resource));
        auto fs_mem = ct_renderer_a0.alloc(shader_blob::fs_size(resource));

        memcpy(vs_mem->data, (resource + 1), resource->vs_size);
        memcpy(fs_mem->data, ((char *) (resource + 1)) + resource->vs_size,
               resource->fs_size);

        auto vs_shader = ct_renderer_a0.create_shader(vs_mem);
        auto fs_shader = ct_renderer_a0.create_shader(fs_mem);
        program = ct_renderer_a0.create_program(vs_shader, fs_shader, true);

    }

    ct_cdb_obj_o *writer = ct_cdb_a0.write_begin(obj);
    ct_cdb_a0.set_uint64(writer, SHADER_PROP, program.idx);
    ct_cdb_a0.write_commit(writer);
}

static void offline(uint64_t name,
                    struct ct_cdb_obj_t *obj) {
    CT_UNUSED(name);

    const uint64_t program = ct_cdb_a0.read_uint64(obj, SHADER_PROP, 0);
    ct_renderer_a0.destroy_program({.idx=(uint16_t)program});
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

    ct_resource_a0.register_type("shader", callback);
    shadercompiler_init(api);

    return 1;
}

void shader_shutdown() {
}

ct_shader shader_get(ct_cdb_obj_t* shader) {
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
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_renderer_a0);
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