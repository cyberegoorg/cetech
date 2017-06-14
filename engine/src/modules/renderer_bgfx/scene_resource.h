#ifndef CETECH_SCENE_RESOURCE_H
#define CETECH_SCENE_RESOURCE_H


#include <cetech/core/path.h>

static const bgfx_texture_handle_t null_texture = {0};


void *_scene_resource_loader(struct vio *input,
                             struct allocator *allocator) {
    const int64_t size = vio_api_v0.size(input);
    char *data = CETECH_ALLOCATE(allocator, char, size);
    vio_api_v0.read(input, data, 1, size);

    return data;
}

void _scene_resource_unloader(void *new_data,
                              struct allocator *allocator) {
    CETECH_DEALLOCATE(allocator, new_data);
}

void _scene_resource_online(uint64_t name,
                            void *data) {
    struct scene_blob *resource = (scene_blob *) data;

    uint64_t *geom_name = scene_blob_geom_name(resource);
    uint32_t *ib_offset = scene_blob_ib_offset(resource);
    uint32_t *vb_offset = scene_blob_vb_offset(resource);
    bgfx_vertex_decl_t *vb_decl = scene_blob_vb_decl(resource);
    uint32_t *ib_size = scene_blob_ib_size(resource);
    uint32_t *vb_size = scene_blob_vb_size(resource);
    uint32_t *ib = scene_blob_ib(resource);
    uint8_t *vb = scene_blob_vb(resource);

    struct scene_instance *instance = _init_scene_instance(name);

    for (int i = 0; i < resource->geom_count; ++i) {
        bgfx_vertex_buffer_handle_t bvb = bgfx_create_vertex_buffer(
                bgfx_make_ref((const void *) &vb[vb_offset[i]], vb_size[i]),
                &vb_decl[i], BGFX_BUFFER_NONE);

        bgfx_index_buffer_handle_t bib = bgfx_create_index_buffer(
                bgfx_make_ref((const void *) &ib[ib_offset[i]],
                              sizeof(uint32_t) * ib_size[i]),
                BGFX_BUFFER_INDEX32);

        uint8_t idx = (uint8_t) array::size(instance->vb);
        map::set(instance->geom_map, geom_name[i], idx);

        array::push_back(instance->size, ib_size[i]);
        array::push_back(instance->vb, bvb);
        array::push_back(instance->ib, bib);
    }
}

void _scene_resource_offline(uint64_t name,
                             void *data) {
    _destroy_scene_instance(name);
}

void *_scene_resource_reloader(uint64_t name,
                               void *old_data,
                               void *new_data,
                               struct allocator *allocator) {
    _scene_resource_offline(name, old_data);
    _scene_resource_online(name, new_data);

    CETECH_DEALLOCATE(allocator, old_data);

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
