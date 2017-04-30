#ifndef CETECH_TEXTURE_RESOURCE_H
#define CETECH_TEXTURE_RESOURCE_H

static const bgfx_texture_handle_t null_texture = {0};


void *_texture_resource_loader(struct vio *input,
                               struct allocator *allocator) {
    const int64_t size = vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    vio_read(input, data, 1, size);

    return data;
}

void _texture_resource_unloader(void *new_data,
                                struct allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
}

void _texture_resource_online(stringid64_t name,
                              void *data) {
    struct texture *resource = data;

    const bgfx_memory_t *mem = bgfx_copy((resource + 1), resource->size);
    bgfx_texture_handle_t texture = bgfx_create_texture(mem, BGFX_TEXTURE_NONE,
                                                        0, NULL);

    MAP_SET(bgfx_texture_handle_t, &_G.handler_map, name.id, texture);
}


void _texture_resource_offline(stringid64_t name,
                               void *data) {
    bgfx_texture_handle_t texture = MAP_GET(bgfx_texture_handle_t,
                                            &_G.handler_map, name.id,
                                            null_texture);

    if (texture.idx == null_texture.idx) {
        return;
    }

    bgfx_destroy_texture(texture);

}

void *_texture_resource_reloader(stringid64_t name,
                                 void *old_data,
                                 void *new_data,
                                 struct allocator *allocator) {
    _texture_resource_offline(name, old_data);
    _texture_resource_online(name, new_data);

    CEL_DEALLOCATE(allocator, old_data);

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
