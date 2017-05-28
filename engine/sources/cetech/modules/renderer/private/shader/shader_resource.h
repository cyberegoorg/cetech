#ifndef CETECH_SHADER_RESOURCE_H
#define CETECH_SHADER_RESOURCE_H


#include <cetech/core/os/path.h>

static const bgfx_program_handle_t null_program = {0};


void *_shader_resource_loader(struct vio *input,
                              struct allocator *allocator) {
    const int64_t size = vio_api_v0.size(input);
    char *data = CETECH_ALLOCATE(allocator, char, size);
    vio_api_v0.read(input, data, 1, size);

    return data;
}

void _shader_resource_unloader(void *new_data,
                               struct allocator *allocator) {
    CETECH_DEALLOCATE(allocator, new_data);
}

void _shader_resource_online(uint64_t name,
                             void *data) {
    struct shader *resource = data;

    const bgfx_memory_t *vs_mem = bgfx_alloc(resource->vs_size);
    const bgfx_memory_t *fs_mem = bgfx_alloc(resource->fs_size);

    memcpy(vs_mem->data, (resource + 1), resource->vs_size);
    memcpy(fs_mem->data, ((char *) (resource + 1)) + resource->vs_size,
                resource->fs_size);

    bgfx_shader_handle_t vs_shader = bgfx_create_shader(vs_mem);
    bgfx_shader_handle_t fs_shader = bgfx_create_shader(fs_mem);

    bgfx_program_handle_t program = bgfx_create_program(vs_shader, fs_shader,
                                                        1);

    MAP_SET(bgfx_program_handle_t, &_G.handler_map, name, program);
}

void _shader_resource_offline(uint64_t name,
                              void *data) {

    bgfx_program_handle_t program = MAP_GET(bgfx_program_handle_t,
                                            &_G.handler_map, name,
                                            null_program);

    if (program.idx == null_program.idx) {
        return;
    }

    bgfx_destroy_program(program);

    MAP_REMOVE(bgfx_program_handle_t, &_G.handler_map, name);
}

void *_shader_resource_reloader(uint64_t name,
                                void *old_data,
                                void *new_data,
                                struct allocator *allocator) {
    _shader_resource_offline(name, old_data);
    _shader_resource_online(name, new_data);

    CETECH_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t shader_resource_callback = {
        .loader = _shader_resource_loader,
        .unloader =_shader_resource_unloader,
        .online =_shader_resource_online,
        .offline =_shader_resource_offline,
        .reloader = _shader_resource_reloader
};


#endif //CETECH_SHADER_RESOURCE_H
