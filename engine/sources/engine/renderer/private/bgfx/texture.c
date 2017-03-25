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

void *texture_resource_loader(struct vio *input,
                              struct cel_allocator *allocator) {
    const i64 size = cel_vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    cel_vio_read(input, data, 1, size);

    return data;
}

void texture_resource_unloader(void *new_data,
                               struct cel_allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
}

void texture_resource_online(stringid64_t name,
                             void *data) {
    struct texture *resource = data;

    const bgfx_memory_t *mem = bgfx_copy((resource + 1), resource->size);
    bgfx_texture_handle_t texture = bgfx_create_texture(mem, BGFX_TEXTURE_NONE,
                                                        0, NULL);

    MAP_SET(bgfx_texture_handle_t, &_G.handler_map, name.id, texture);
}

static const bgfx_texture_handle_t null_texture = {0};

void texture_resource_offline(stringid64_t name,
                              void *data) {
    bgfx_texture_handle_t texture = MAP_GET(bgfx_texture_handle_t,
                                            &_G.handler_map, name.id,
                                            null_texture);

    if (texture.idx == null_texture.idx) {
        return;
    }

    bgfx_destroy_texture(texture);

}

void *texture_resource_reloader(stringid64_t name,
                                void *old_data,
                                void *new_data,
                                struct cel_allocator *allocator) {
    texture_resource_offline(name, old_data);
    texture_resource_online(name, new_data);

    CEL_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t texture_resource_callback = {
        .loader = texture_resource_loader,
        .unloader =texture_resource_unloader,
        .online =texture_resource_online,
        .offline =texture_resource_offline,
        .reloader = texture_resource_reloader
};


//==============================================================================
// Interface
//==============================================================================

int texture_resource_init() {
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

void texture_resource_shutdown() {
    MAP_DESTROY(bgfx_texture_handle_t, &_G.handler_map);

    _G = (struct G) {0};
}

bgfx_texture_handle_t texture_resource_get(stringid64_t name) {
    ResourceApiV0.get(_G.type, name); // TODO: only for autoload

    return MAP_GET(bgfx_texture_handle_t, &_G.handler_map, name.id,
                   null_texture);
}