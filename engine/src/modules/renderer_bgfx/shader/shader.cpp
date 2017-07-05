//==============================================================================
// Include
//==============================================================================

#include <cstdio>

#include <cetech/celib/allocator.h>
#include <cetech/celib/map.inl>
#include <cetech/celib/string_stream.h>

#include <cetech/kernel/application.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/log.h>

#include <cetech/kernel/hash.h>
#include <cetech/kernel/os.h>

#include <cetech/modules/resource.h>

#include <bgfx/bgfx.h>

#include "shader_blob.h"

using namespace cetech;
using namespace string_stream;

namespace shader_compiler {
    int init(api_v0 *api);
}

//==============================================================================
// GLobals
//==============================================================================

#define _G ShaderResourceGlobals
struct shader_blobResourceGlobals {
    Map<bgfx::ProgramHandle> handler_map;
    uint64_t type;
} ShaderResourceGlobals;


CETECH_DECL_API(memory_api_v0)
CETECH_DECL_API(resource_api_v0)
CETECH_DECL_API(app_api_v0)
CETECH_DECL_API(os_path_v0)
CETECH_DECL_API(os_vio_api_v0)
CETECH_DECL_API(os_process_api_v0)
CETECH_DECL_API(log_api_v0)
CETECH_DECL_API(hash_api_v0)


//==============================================================================
// Resource
//==============================================================================

namespace shader_resource {

    static const bgfx::ProgramHandle null_program = {0};


    void *loader(struct os_vio *input,
                 struct allocator *allocator) {
        const int64_t size = os_vio_api_v0.size(input);
        char *data = CETECH_ALLOCATE(allocator, char, size);
        os_vio_api_v0.read(input, data, 1, size);

        return data;
    }

    void unloader(void *new_data,
                  struct allocator *allocator) {
        CETECH_FREE(allocator, new_data);
    }

    void online(uint64_t name,
                void *data) {
        auto resource = shader_blob::get(data);

        auto vs_mem = bgfx::alloc(shader_blob::vs_size(resource));
        auto fs_mem = bgfx::alloc(shader_blob::fs_size(resource));

        memcpy(vs_mem->data, (resource + 1), resource->vs_size);
        memcpy(fs_mem->data, ((char *) (resource + 1)) + resource->vs_size,
               resource->fs_size);

        auto vs_shader = bgfx::createShader(vs_mem);
        auto fs_shader = bgfx::createShader(fs_mem);
        auto program = bgfx::createProgram(vs_shader, fs_shader, 1);

        map::set(_G.handler_map, name, program);
    }

    void offline(uint64_t name,
                 void *data) {

        auto program = map::get(_G.handler_map, name, null_program);

        if (program.idx == null_program.idx) {
            return;
        }

        bgfx::destroyProgram(program);

        map::remove(_G.handler_map, name);
    }

    void *reloader(uint64_t name,
                   void *old_data,
                   void *new_data,
                   struct allocator *allocator) {
        offline(name, old_data);
        online(name, new_data);

        CETECH_FREE(allocator, old_data);

        return new_data;
    }

    static const resource_callbacks_t callback = {
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
    int shader_init(struct api_v0 *api) {
        CETECH_GET_API(api, memory_api_v0);
        CETECH_GET_API(api, resource_api_v0);
        CETECH_GET_API(api, app_api_v0);
        CETECH_GET_API(api, os_path_v0);
        CETECH_GET_API(api, os_vio_api_v0);
        CETECH_GET_API(api, os_process_api_v0);
        CETECH_GET_API(api, log_api_v0);
        CETECH_GET_API(api, hash_api_v0);

        _G = {0};

        _G.type = hash_api_v0.id64_from_str("shader");

        _G.handler_map.init(memory_api_v0.main_allocator());

        resource_api_v0.register_type(_G.type,
                                      shader_resource::callback);
#ifdef CETECH_CAN_COMPILE
        shader_compiler::init(api);
#endif

        return 1;
    }

    void shader_shutdown() {
        _G.handler_map.destroy();
    }

    bgfx::ProgramHandle shader_get(uint64_t name) {
        auto resource = shader_blob::get(resource_api_v0.get(_G.type, name));
        return map::get(_G.handler_map, name, shader_resource::null_program);
    }
}
    