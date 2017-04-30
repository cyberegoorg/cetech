#ifndef CETECH_SCENE_RESOURCE_H
#define CETECH_SCENE_RESOURCE_H


static const bgfx_texture_handle_t null_texture = {0};


void *_scene_resource_loader(struct vio *input,
                             struct allocator *allocator) {
    const int64_t size = vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    vio_read(input, data, 1, size);

    return data;
}

void _scene_resource_unloader(void *new_data,
                              struct allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
}

void _scene_resource_online(stringid64_t name,
                            void *data) {
    struct scene_blob *resource = data;

    stringid64_t *geom_name = scene_blob_geom_name(resource);
    uint32_t *ib_offset = scene_blob_ib_offset(resource);
    uint32_t *vb_offset = scene_blob_vb_offset(resource);
    bgfx_vertex_decl_t *vb_decl = scene_blob_vb_decl(resource);
    uint32_t *ib_size = scene_blob_ib_size(resource);
    uint32_t *vb_size = scene_blob_vb_size(resource);
    uint32_t *ib = scene_blob_ib(resource);
    uint8_t *vb = scene_blob_vb(resource);

    struct scene_instance instance = {0};
    _init_scene_instance(&instance);

    for (int i = 0; i < resource->geom_count; ++i) {
        bgfx_vertex_buffer_handle_t bvb = bgfx_create_vertex_buffer(
                bgfx_make_ref((const void *) &vb[vb_offset[i]], vb_size[i]),
                &vb_decl[i], BGFX_BUFFER_NONE);

        bgfx_index_buffer_handle_t bib = bgfx_create_index_buffer(
                bgfx_make_ref((const void *) &ib[ib_offset[i]],
                              sizeof(uint32_t) * ib_size[i]), BGFX_BUFFER_INDEX32);

        uint32_t idx = ARRAY_SIZE(&instance.vb);
        MAP_SET(uint8_t, &instance.geom_map, geom_name[i].id, idx);

        ARRAY_PUSH_BACK(uint32_t, &instance.size, ib_size[i]);
        ARRAY_PUSH_BACK(bgfx_vertex_buffer_handle_t, &instance.vb, bvb);
        ARRAY_PUSH_BACK(bgfx_index_buffer_handle_t, &instance.ib, bib);
    }

    MAP_SET(scene_instance, &_G.scene_instance, name.id, instance);
}

void _scene_resource_offline(stringid64_t name,
                             void *data) {
    struct scene_instance instance = MAP_GET(scene_instance, &_G.scene_instance,
                                             name.id,
                                             (struct scene_instance) {0});
    _destroy_scene_instance(&instance);
    MAP_REMOVE(scene_instance, &_G.scene_instance, name.id);
}

void *_scene_resource_reloader(stringid64_t name,
                               void *old_data,
                               void *new_data,
                               struct allocator *allocator) {
    _scene_resource_offline(name, old_data);
    _scene_resource_online(name, new_data);

    CEL_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t scene_resource_callback = {
        .loader = _scene_resource_loader,
        .unloader =_scene_resource_unloader,
        .online =_scene_resource_online,
        .offline =_scene_resource_offline,
        .reloader = _scene_resource_reloader
};


#endif //CETECH_SCENE_RESOURCE_H
