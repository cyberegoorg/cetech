//==============================================================================
// Include
//==============================================================================

#include <cstdio>

#include "celib/allocator.h"
#include "celib/map.inl"
#include "celib/buffer.inl"

#include "cetech/os/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/log/log.h"

#include "cetech/hashlib/hashlib.h"
#include "cetech/machine/machine.h"

#include "cetech/resource/resource.h"

#include <bgfx/bgfx.h>
#include <cetech/module/module.h>
#include <cetech/renderer/shader.h>

#include "shader_blob.h"
#include "cetech/os/path.h"
#include "cetech/os/process.h"
#include "cetech/os/vio.h"

using namespace celib;
using namespace buffer;

namespace shader_compiler {
    int init(ct_api_a0 *api);
}

//==============================================================================
// GLobals
//==============================================================================

#define _G ShaderResourceGlobals
struct _G {
    Map<ct_shader> handler_map;
    uint64_t type;
} _G;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_process_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);

//==============================================================================
// Resource
//==============================================================================

namespace shader_resource {

    static const ct_shader null_program = BGFX_INVALID_HANDLE;


    void *loader(ct_vio *input,
                 cel_alloc *allocator) {

        const int64_t size = input->size(input->inst);
        char *data = CEL_ALLOCATE(allocator, char, size);
        input->read(input->inst, data, 1, size);
        return data;
    }

    void unloader(void *new_data,
                  cel_alloc *allocator) {
        CEL_FREE(allocator, new_data);
    }

    void online(uint64_t name,
                void *data) {
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

        map::set(_G.handler_map, name, {program.idx});
    }

    void offline(uint64_t name,
                 void *data) {
        CEL_UNUSED(data);

        auto program = map::get(_G.handler_map, name, null_program);

        if (program.idx == null_program.idx) {
            return;
        }

        bgfx::destroy((bgfx::ProgramHandle) {program.idx});

        map::remove(_G.handler_map, name);
    }

    void *reloader(uint64_t name,
                   void *old_data,
                   void *new_data,
                   cel_alloc *allocator) {
        offline(name, old_data);
        online(name, new_data);

        CEL_FREE(allocator, old_data);

        return new_data;
    }

    static const ct_resource_callbacks_t callback = {
            .loader = loader,
            .unloader =unloader,
            .online = online,
            .offline = offline,
            .reloader = reloader
    };

}

//==============================================================================
// Interface
//==============================================================================




namespace shader {
    int shader_init(ct_api_a0 *api) {
        _G = {};

        _G.type = CT_ID64_0("shader");

        _G.handler_map.init(ct_memory_a0.main_allocator());

        ct_resource_a0.register_type(_G.type,
                                     shader_resource::callback);
        shader_compiler::init(api);

        return 1;
    }

    void shader_shutdown() {
        _G.handler_map.destroy();
    }

    ct_shader shader_get(uint64_t name) {
        shader_blob::get(ct_resource_a0.get(_G.type, name));
        return map::get(_G.handler_map, name, shader_resource::null_program);
    }
}

static ct_shader_a0 shader_api = {
        .get = shader::shader_get
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
        },
        {
            CEL_UNUSED(reload);
            _init_api(api);
            shader::shader_init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            shader::shader_shutdown();
        }
)