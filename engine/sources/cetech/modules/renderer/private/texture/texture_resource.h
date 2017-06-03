#ifndef CETECH_TEXTURE_RESOURCE_H
#define CETECH_TEXTURE_RESOURCE_H

#include <cetech/core/os/path.h>

static const bgfx_texture_handle_t null_texture = {0};


void *_texture_resource_loader(struct vio *input,
                               struct allocator *allocator) {
    const int64_t size = vio_api_v0.size(input);
    char *data = CETECH_ALLOCATE(allocator, char, size);
    vio_api_v0.read(input, data, 1, size);

    return data;
}

void _texture_resource_unloader(void *new_data,
                                struct allocator *allocator) {
    CETECH_DEALLOCATE(allocator, new_data);
}

void _texture_resource_online(uint64_t name,
                              void *data) {
    struct texture *resource = (texture *) data;

    const bgfx_memory_t *mem = bgfx_copy((resource + 1), resource->size);
    bgfx_texture_handle_t texture = bgfx_create_texture(mem, BGFX_TEXTURE_NONE,
                                                        0, NULL);

    MAP_SET(bgfx_texture_handle_t, &_G.handler_map, name, texture);
}


void _texture_resource_offline(uint64_t name,
                               void *data) {
    bgfx_texture_handle_t texture = MAP_GET(bgfx_texture_handle_t,
                                            &_G.handler_map, name,
                                            null_texture);

    if (texture.idx == null_texture.idx) {
        return;
    }

    bgfx_destroy_texture(texture);

}

void *_texture_resource_reloader(uint64_t name,
                                 void *old_data,
                                 void *new_data,
                                 struct allocator *allocator) {
    _texture_resource_offline(name, old_data);
    _texture_resource_online(name, new_data);

    CETECH_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t texture_resource_callback = {
        .loader = _texture_resource_loader,
        .unloader =_texture_resource_unloader,
        .online =_texture_resource_online,
        .offline =_texture_resource_offline,
        .reloader = _texture_resource_reloader
};


#endif //CETECH_TEXTURE_RESOURCE_H
