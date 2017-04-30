#ifndef CETECH_MATERIAL_RESORCE_H
#define CETECH_MATERIAL_RESORCE_H

static const bgfx_program_handle_t null_program = {0};

void *_material_resource_loader(struct vio *input,
                                struct allocator *allocator) {
    const int64_t size = vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    vio_read(input, data, 1, size);

    return data;
}

void _material_resource_unloader(void *new_data,
                                 struct allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
}

void _material_resource_online(stringid64_t name,
                               void *data) {
}

void _material_resource_offline(stringid64_t name,
                                void *data) {
}

void *_material_resource_reloader(stringid64_t name,
                                  void *old_data,
                                  void *new_data,
                                  struct allocator *allocator) {
    _material_resource_offline(name, old_data);
    _material_resource_online(name, new_data);

    CEL_DEALLOCATE(allocator, old_data);
    return new_data;
}

static const resource_callbacks_t material_resource_callback = {
        .loader = _material_resource_loader,
        .unloader =_material_resource_unloader,
        .online =_material_resource_online,
        .offline =_material_resource_offline,
        .reloader = _material_resource_reloader
};

#endif //CETECH_MATERIAL_RESORCE_H
